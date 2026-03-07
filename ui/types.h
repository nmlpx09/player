#pragma once

#include <filesystem>
#include <vector>

namespace NUI {

enum class ECommands {
    EMPTY,
    PLAY,
    STOP,
    UP,
    DOWN,
    EXIT,
    ENTER,
    QUIT
};

using TFiles = std::vector<std::filesystem::path>;

}
