#include "cui.h"

namespace NUI {

TCUI::TCUI(std::size_t winWidth, std::size_t winHight)
: WinWidth(winWidth)
, WinHight(winHight) { }

void TCUI::Init() noexcept {
    initscr();
    noecho();
    keypad(stdscr, true);
    curs_set(false);

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);

    WinMusicList = newwin(WinHight + 2, WinWidth, 0, 0);
    WinMain = newwin(3, WinWidth, WinHight + 2, 0);

    wbkgd(WinMain, COLOR_PAIR(1));

    box(WinMusicList, 0, 0);

    refresh();
}

void TCUI::Close() noexcept {
    if(WinMusicList != nullptr) {
        delwin(WinMusicList);
    }

    if (WinMain != nullptr) {
        delwin(WinMain);
    }

    endwin();
}

void TCUI::MusicListOut(
    const std::vector<std::filesystem::path>& directories,
    std::size_t current
) noexcept {
    if (WinMusicList == nullptr) {
        return;
    }

    std::size_t begin = 0;
    std::size_t end = WinHight;

    if (current >= WinHight) {
        begin = current - WinHight + 1;
        end = current + 1;
    }

    wclear(WinMusicList);
    wattron(WinMusicList, COLOR_PAIR(2));
    box(WinMusicList, 0, 0);
    mvwprintw(WinMusicList, 0, 1, "%s", MusicListTitle.c_str());

    for (std::size_t i = begin, j = 1; i != end; ++j, ++i) {
        if (current == i) {
            wattron(WinMusicList, COLOR_PAIR(1));
        } else {
            wattron(WinMusicList, COLOR_PAIR(2));
        }

        auto message = directories[i].filename().string();

        if (message.size() >= WinWidth ) {
            message.resize(WinWidth);
        }

        mvwprintw(WinMusicList, j, 1, "%s", message.c_str());
    }

    wrefresh(WinMusicList);
}

void TCUI::MainOut(const std::string& message) noexcept {
    if (WinMain == nullptr) {
        return;
    }
    wclear(WinMain);
    mvwprintw(WinMain, 0, 0, "%s", message.c_str());
    wrefresh(WinMain);
}

}
