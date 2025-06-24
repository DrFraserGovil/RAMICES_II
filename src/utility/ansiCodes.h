#include <string>

namespace ANSI {
    const std::string CSI = "\x1b["; // Control Sequence Introducer
    const std::string CLEAR_LINE = CSI + "2K"; // Erase entire current line
    const std::string CURSOR_UP = CSI + "A"; // Move cursor up 1 line
    const std::string CURSOR_TO_COL1 = CSI + "1G"; // Move cursor to column 1
    const std::string RESET_FORMAT = CSI + "0m"; // Reset all formatting
	const std::string COLOR = "\033[3"; //color sequence indicator
	const std::string BLUE_FONT = COLOR + "4m";
	const std::string WHITE_FONT = COLOR + "7m";
	const std::string PURPLE_FONT = COLOR + "8;5;141m";
	const std::string RED_FONT = COLOR + "1m";
}