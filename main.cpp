#include "config.h"

#include <common/context.h>
#include <common/types.h>
#include <read/flac/flac.h>
#ifdef ALSA
#include <write/alsa/alsa.h>
#else
#include <write/pulse/pulse.h>
#endif

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>


void Write(TContextPtr ctx, NWrite::TWritePtr write) {
    try {
        auto getPayload = [=] () {
            auto payload = ctx->GetPayload();
            ctx->ReadNotify();
            return payload;
        };

        while (true) {
            ctx->WriteWait();

            if (ctx->IsStop()) {
                break;
            }

            if (auto ec = write->Write(getPayload); ec) {
                std::cerr << "write error: " + ec.message() << std::endl;
                break;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "write exception:" << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "write unknown exception" << std::endl;
    }
    ctx->Stop();
}

void Read(TContextPtr ctx, TFiles files) {
    try {
        for (const auto& file: files) {
            if (ctx->IsStop()) {
                break;
            }

            if (!TFormatPermited::Format.contains(file.extension())) {
                continue;
            }

            TFormat format;

            NRead::TReadPtr read = std::make_unique<NRead::TFlac>();
            if (auto result = read->Init(file.string()); !result) {
                std::cerr << "read init error: " + result.error().message() << std::endl;
                continue;
            } else {
                format = result.value();
            }


            std::cout << file.filename().string() + "; " + std::to_string(format.SampleRate)
                + "hz " + std::to_string(format.BitsPerSample) + "bps "
                + std::to_string(format.NumChannels) + "ch\n";

            auto storePayload = [=, &format] (TData data) {
                ctx->StorePayload(std::make_pair(format, std::move(data)));
                ctx->WriteNotify();
            };

            while (true) {
                ctx->ReadWait();

                if (ctx->IsStop()) {
                    break;
                }

                if (auto result = read->Read(storePayload); !result) {
                    std::cerr << "read error: " + result.error().message() << std::endl;
                    break;
                } else if (!result.value()) {
                    break;
                }
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "read exception:" << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "read unknown exception" << std::endl;
    }
    ctx->Stop();
}

TFiles GetFiles(const TFiles::value_type& path) {
    TFiles files;
    try {

        if (std::filesystem::is_directory(path)) {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (TFormatPermited::Format.contains(entry.path().extension())) {
                    files.push_back(entry.path());
                }
            }
        } else {
            if (TFormatPermited::Format.contains(path.extension())) {
                files.push_back(path);
            }
        }
        std::sort(files.begin(), files.end());
    } catch(const std::exception& ex) {
        std::cerr << "get files error:" << ex.what() << std::endl;
    }

    return files;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "run: play dir" << std::endl;
        return 1;
    }

    auto dir = std::string{argv[1]};
    auto files = GetFiles(dir);

    if (files.empty()) {
        std::cerr << "empty directory: " << dir << std::endl;
        return 1;
    }

    std::string device;
    if (argc == 3) {
        device = std::string{argv[2]};
    } else {
        device = DEVICE;
    }

    auto ctx = std::make_shared<TContext>();

    NWrite::TWritePtr write = std::make_unique<NWrite::TWrite>(device);

    if (auto ec = write->Init(TFormat{}); ec) {
        std::cerr << "init device error: " << device << std::endl;
        return 1;
    }

    std::thread tWrite(Write, ctx, std::move(write));
    std::thread tRead(Read, ctx, std::move(files));

    tRead.join();
    tWrite.join();

    return 0;
}
