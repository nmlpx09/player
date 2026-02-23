#include <filesystem>
#include <vector>

namespace NUI {

enum class ECommands {
	EMPTY,
	PLAY,
	STOP,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	QUIT
};

using TFiles = std::vector<std::filesystem::path>;

}
