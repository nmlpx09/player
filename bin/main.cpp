#include "config.h"

#include <common/context.h>
#include <read/flac.h>
#include <ui/cui.h>

#ifdef ALSA
#include <write/alsa.h>
#else
#include <write/pulse.h>
#endif

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <optional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

void Write(TContextPtr ctx, NWrite::TWritePtr write, NUI::TUIPtr ui) noexcept {
    auto popQueue = [=] () noexcept -> std::optional<std::pair<TFormat, TData>> {
        auto [date, format, buffer] = ctx->Queue.front();
        ctx->Queue.pop_front();
        ctx->ReadCv.notify_one();

        std::this_thread::sleep_until(date);

        return std::make_optional(std::make_pair(std::move(format), std::move(buffer)));
    };

    while (!ctx->IsEnd()) {
        std::unique_lock<std::mutex> ulock{ctx->Mutex};
        ctx->WriteCv.wait(ulock, [ctx] { return !ctx->Queue.empty() || ctx->IsEnd(); });
        ulock.unlock();
        
        if (ctx->IsEnd()) {
            return;
        }

        if (auto ec = write->Write(popQueue); ec) {
            std::string error = "write error: " + ec.message();
            ui->StatusDraw(error);
            break;
        }
    }
    ctx->Stop();
}

void Read(TContextPtr ctx, std::vector<std::filesystem::path> files, NUI::TUIPtr ui) noexcept {
    const auto delta = std::chrono::milliseconds(500);

    std::size_t current = 0;
    for (const auto& file: files) {
        if (ctx->IsEnd()) {
            break;
        }

        auto time = std::chrono::steady_clock::now() + delta;

        TFormat format;

        auto pushQueue = [=, &time, &format] (TData data) noexcept {
            ctx->Queue.emplace_back(std::make_tuple(time, format, std::move(data)));
            ctx->WriteCv.notify_one();

            time += delta;
        };

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

        while (!ctx->IsEnd()) {
            std::unique_lock<std::mutex> ulock{ctx->Mutex};
            ctx->ReadCv.wait(ulock, [ctx] { return ctx->Queue.empty() || ctx->IsEnd(); });
            ulock.unlock();

            if (ctx->IsEnd()) {
                break;
            }

            if (auto result = read->Read(pushQueue); !result) {
                std::string error = "read error: {} " + result.error().message();
                ui->StatusDraw(error);
                break;
            } else if (!result.value()) {
                break;
            }
        }
    }

    ui->StatusDraw("");
    ctx->Stop();
} 

std::vector<std::filesystem::path> GetDirList(const std::filesystem::path& path) {
    std::vector<std::filesystem::path> directories;

    if (std::filesystem::is_directory(path)) {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (std::filesystem::is_directory(entry)) {
                directories.push_back(entry.path());
            }
        }
    }
    std::sort(directories.begin(), directories.end());
    return directories;
}

std::vector<std::filesystem::path> GetFileList(const std::filesystem::path& path) {
    std::vector<std::filesystem::path> files;

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
    return files;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "run: play dir" << std::endl;
        return 1;
    }

    auto basePath = std::string{argv[1]};

    auto directories = GetDirList(basePath);

    if (directories.empty()) {
        std::cerr << "opem music list error: " << basePath << std::endl;
        return 1;
    }

    std::string device;
    if (argc == 3) {
        device = std::string{argv[2]};
    } else {
        device = DEVICE;
    }

    NWrite::TWritePtr write = std::make_shared<NWrite::TWrite>(device);

    if (auto ec = write->Init(TFormat{}); ec) {
        std::cerr << "init device error: " << device << std::endl;
        return 1;
    }

    std::size_t current = 0;

    auto ui = std::make_shared<NUI::TCUI>(WIN_WIDTH, WIN_HIGHT);

    ui->Init();
    ui->ListDraw(directories, current);
    ui->StatusDraw("");

    auto ctx = std::make_shared<TContext>();

    auto input = 0;
    while ((input = getch()) != ERR) {
        if (input == 'q') {
            ctx->Stop();
            break;
        } else if (input == KEY_UP && ctx->IsEnd()) {
            current = current == 0 ? 0 : current - 1;
            ui->ListDraw(directories, current);
        } else if (input == KEY_DOWN && ctx->IsEnd()) {
            current = std::min(directories.size() - 1, current + 1);
            ui->ListDraw(directories, current);
        } else if (input == 'p' && ctx->IsEnd()) {
            ctx->Start();

            auto files = GetFileList(directories[current]);

            std::thread tWrite(Write, ctx, write, ui);
            std::thread tRead(Read, ctx, std::move(files), ui);

            tWrite.detach();
            tRead.detach();
        } else if (input == 's') {
            ui->ListDraw(directories, current);
            ctx->Stop();
        }
    }

    ui->Close();

    return 0;
}
