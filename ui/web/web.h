#pragma once

#include <ui/interface.h>
#include <ui/types.h>

#include <cstdint>
#include <mutex>
#include <string>

namespace NUI {

struct TWeb : TInterface {
public:
    TWeb(std::size_t winWidth, std::size_t winHight, std::uint16_t port);
    TWeb(const TWeb&) = delete;
    TWeb(TWeb&&) noexcept = delete;
    ~TWeb();
    TWeb& operator=(const TWeb&) = delete;
    TWeb& operator=(TWeb&&) = delete;

    void ListDraw(const TFiles& filesList, std::size_t currentPos) noexcept override;
    void StatusDraw(const std::string& message) noexcept override;
    void StatusClean() noexcept override;
    std::error_code Init() noexcept override;

    std::expected<ECommands, std::error_code> GetCommand() noexcept override;
private:
    std::string GetList() noexcept;
    std::string GetStatus() noexcept;
private:
    std::int32_t Sockfd = -1;
    std::size_t RequestSize = 200;

    std::size_t WinWidth = 60;
    std::size_t WinHight = 10;
    std::uint16_t Port = 0;

    std::string List;
    std::string Status;

    std::mutex Mutex;
};

}
