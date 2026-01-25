#include "interface.h"

#include <ncurses.h>

#include <string>

namespace NUI {

struct TCUI : TInterface {
public:
    TCUI(std::size_t winWidth, std::size_t winHight);
    TCUI(const TCUI&) = delete;
    TCUI(TCUI&&) noexcept = delete;
    ~TCUI() = default;
    TCUI& operator=(const TCUI&) = delete;
    TCUI& operator=(TCUI&&) = delete;

    void ListDraw(const std::vector<std::filesystem::path>& musicList, std::size_t currentPos) noexcept override;
    void StatusDraw(const std::string& message) noexcept override;
    void Init() noexcept override;
    void Close() noexcept override;
private:
    WINDOW* WinMusicList = nullptr;
    WINDOW* WinMain = nullptr;

    std::size_t WinWidth = 60;
    std::size_t WinHight = 10;
};

}
