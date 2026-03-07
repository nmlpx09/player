#include "config.h"

#include <common/context.h>
#include <common/types.h>
#include <read/flac/flac.h>
#include <ui/web/web.h>
#include <write/alsa/alsa.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>

void Write(TContextPtr ctx, NWrite::TWritePtr write, NUI::TUIPtr ui) noexcept {
    auto getPayload = [=] () noexcept {
        auto payload = ctx->GetPayload();
        ctx->ReadNotify();

        std::this_thread::sleep_until(std::get<0>(payload));

        return payload;
    };

    while (true) {
        ctx->WriteWait();

        if (ctx->IsStop()) {
            break;
        }

        if (auto ec = write->Write(getPayload); ec) {
            std::string error = "write error: " + ec.message();
            ui->StatusDraw(error);
            break;
        }
    }
}

void Read(TContextPtr ctx, TFiles files, NUI::TUIPtr ui) noexcept {
    const auto delta = std::chrono::milliseconds(500);

    std::size_t current = 0;
    for (const auto& file: files) {
        if (ctx->IsStop()) {
            break;
        }

        if (std::filesystem::is_directory(file) || !TFormatPermited::Format.contains(file.extension())) {
            continue;
        }

        auto time = std::chrono::steady_clock::now() + delta;

        TFormat format;

        NRead::TReadPtr read = std::make_unique<NRead::TFlac>();
        if (auto result = read->Init(file.string()); !result) {
            std::string error = "read init error: {} " + result.error().message();
            ui->StatusDraw(error);
            return;
        } else {
            format = result.value();
        }

        std::string currentFile = file.filename().string() + "; " +
            std::to_string(format.SampleRate) + "hz " +
            std::to_string(format.BitsPerSample) + "bps " +
            std::to_string(format.NumChannels) + "ch";

        ui->StatusDraw(currentFile);
        ui->ListDraw(files, current++);

        auto storePayload = [=, &time, &format] (TData data) noexcept {
            ctx->StorePayload(std::make_tuple(time, format, std::move(data)));
            ctx->WriteNotify();

            time += delta;
        };

        while (true) {
            ctx->ReadWait();

            if (ctx->IsStop()) {
                break;
            }

            if (auto result = read->Read(storePayload); !result) {
                std::string error = "read error: {} " + result.error().message();
                ui->StatusDraw(error);
                break;
            } else if (!result.value()) {
                break;
            }
        }
    }

    ui->StatusClean();
}

void BuildFileSystem(TFileSystem& fileSystem, const std::filesystem::path& base, const std::filesystem::path& current) {
    fileSystem.insert({current, {base, {}}});
    auto& files = fileSystem.at(current).second;

    if (std::filesystem::is_directory(current)) {
        for (const auto& entry : std::filesystem::directory_iterator(current)) {
            if (std::filesystem::is_directory(entry)) {
                BuildFileSystem(fileSystem, current, entry);
                files.push_back(entry);
            } else {
                files.push_back(entry);
            }
        }
        std::sort(files.begin(), files.end());
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "run: play dir" << std::endl;
        return 1;
    }

    auto basePath = std::filesystem::path{std::string{argv[1]}};

    TFileSystem fileSystem;
    BuildFileSystem(fileSystem, std::filesystem::path{}, basePath);

    auto parent = fileSystem.at(basePath).first;
    auto files = fileSystem.at(basePath).second;
    std::size_t position = 0;

    NWrite::TWritePtr write = std::make_shared<NWrite::TWrite>(DEVICE);

    if (auto ec = write->Init(TFormat{}); ec) {
        std::cerr << "init device error: " << ec.message() << std::endl;
        return 1;
    }

    auto ui = std::make_shared<NUI::TWeb>(WIN_WIDTH, WIN_HIGHT, PORT);

    if (auto ec = ui->Init(); ec) {
        std::cerr << "init ui error: " << ec.message() << std::endl;
        return 1;
    }

    ui->ListDraw(files, position);
    ui->StatusClean();

    auto ctx = std::make_shared<TContext>();

    while (true) {
        if (auto result = ui->GetCommand(); result) {
            auto command = result.value();
            if (command == NUI::ECommands::QUIT) {
                ctx->Stop();
                break;
            } else if (command == NUI::ECommands::UP && ctx->IsStop()) {
                position = position == 0 ? 0 : position - 1;
                ui->ListDraw(files, position);
            } else if (command == NUI::ECommands::DOWN && ctx->IsStop()) {
                position = std::min(files.size() - 1, position + 1);
                ui->ListDraw(files, position);
            } else if (command == NUI::ECommands::ENTER && ctx->IsStop()) {
                auto newPath = files[position];
                if (std::filesystem::is_directory(newPath)) {
                    parent = fileSystem.at(newPath).first;
                    files = fileSystem.at(newPath).second;
                    position = 0;
                    ui->ListDraw(files, position);
                }
            } else if (command == NUI::ECommands::EXIT && ctx->IsStop()) {
                auto newPath = parent;
                if (!newPath.empty()) {
                    parent = fileSystem.at(newPath).first;
                    files = fileSystem.at(newPath).second;
                    position = 0;
                    ui->ListDraw(files, position);
                }
            } else if (command == NUI::ECommands::PLAY && ctx->IsStop()) {
                TFiles filesForPlay;
                if (std::filesystem::is_directory(files[position])) {
                    filesForPlay = fileSystem.at(files[position]).second;
                } else {
                    filesForPlay.push_back(files[position]);
                }
                ctx->Start();
                std::thread tRead(Read, ctx, std::move(filesForPlay), ui);
                std::thread tWrite(Write, ctx, write, ui);
                tRead.detach();
                tWrite.detach();
            } else if (command == NUI::ECommands::STOP) {
                ui->ListDraw(files, position);
                ctx->Stop();
            }
        } else {
            std::cerr << "read error command : " << result.error().message() << std::endl;
        }
    }

    return 0;
}
