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
    if (WinMusicList != nullptr) {
        delwin(WinMusicList);
        WinMusicList = nullptr;
    }

    if (WinMain != nullptr) {
        delwin(WinMain);
        WinMain = nullptr;
    }

    endwin();
}

void TCUI::ListDraw(
    const TFiles& filesList,
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

    for (std::size_t i = begin, j = 1; i != end; ++j, ++i) {
        if (current == i) {
            wattron(WinMusicList, COLOR_PAIR(1));
        } else {
            wattron(WinMusicList, COLOR_PAIR(2));
        }

        std::string message;
        if (i < filesList.size()) {
            message = filesList[i].filename().string();

            if (message.size() >= WinWidth - 2) {
                message.resize(WinWidth - 2);
            }
        }

        mvwprintw(WinMusicList, j, 1, "%s", message.c_str());
    }

    wrefresh(WinMusicList);
}

void TCUI::StatusDraw(const std::string& message) noexcept {
    if (WinMain == nullptr) {
        return;
    }
    wclear(WinMain);
    mvwprintw(WinMain, 0, 0, "%s", message.c_str());
    wrefresh(WinMain);
}

ECommands TCUI::GetCommand() noexcept {
    auto input = 0;

    while ((input = getch()) != ERR) {
        if (input == 'q') {
            return ECommands::QUIT;
        } else if (input == KEY_UP) {
            return ECommands::UP;
        } else if (input == KEY_DOWN) {
            return ECommands::DOWN;
        } else if (input == KEY_LEFT) {
            return ECommands::LEFT;
        } else if (input == KEY_RIGHT) {
            return ECommands::RIGHT;
        } else if (input == 'p') {
            return ECommands::PLAY;
        } else if (input == 's') {
            return ECommands::STOP;
        }
    }

    return ECommands::EMPTY;
}

}
