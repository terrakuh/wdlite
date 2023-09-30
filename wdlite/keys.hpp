#pragma once

#include <string_view>

namespace wdlite::key {

constexpr std::string_view null = "\uE000";
constexpr std::string_view cancel = "\uE001";
constexpr std::string_view help = "\uE002";
constexpr std::string_view back_space = "\uE003";
constexpr std::string_view tab = "\uE004";
constexpr std::string_view clear = "\uE005";
constexpr std::string_view return_ = "\uE006";
constexpr std::string_view enter = "\uE007";
constexpr std::string_view shift = "\uE008";
constexpr std::string_view control = "\uE009";
constexpr std::string_view alt = "\uE00A";
constexpr std::string_view pause = "\uE00B";
constexpr std::string_view escape = "\uE00C";
constexpr std::string_view space = "\uE00D";
constexpr std::string_view page_up = "\uE00E";
constexpr std::string_view page_down = "\uE00F";
constexpr std::string_view end = "\uE010";
constexpr std::string_view home = "\uE011";
constexpr std::string_view arrow_left = "\uE012";
constexpr std::string_view left = "\uE012";
constexpr std::string_view arrow_up = "\uE013";
constexpr std::string_view up = "\uE013";
constexpr std::string_view arrow_right = "\uE014";
constexpr std::string_view right = "\uE014";
constexpr std::string_view arrow_down = "\uE015";
constexpr std::string_view down = "\uE015";
constexpr std::string_view insert = "\uE016";
constexpr std::string_view delete_ = "\uE017";
constexpr std::string_view semicolon = "\uE018";
constexpr std::string_view equals = "\uE019";

constexpr std::string_view numpad0 = "\uE01A";
constexpr std::string_view numpad1 = "\uE01B";
constexpr std::string_view numpad2 = "\uE01C";
constexpr std::string_view numpad3 = "\uE01D";
constexpr std::string_view numpad4 = "\uE01E";
constexpr std::string_view numpad5 = "\uE01F";
constexpr std::string_view numpad6 = "\uE020";
constexpr std::string_view numpad7 = "\uE021";
constexpr std::string_view numpad8 = "\uE022";
constexpr std::string_view numpad9 = "\uE023";
constexpr std::string_view multiply = "\uE024";
constexpr std::string_view add = "\uE025";
constexpr std::string_view separator = "\uE026";
constexpr std::string_view subtract = "\uE027";
constexpr std::string_view decimal = "\uE028";
constexpr std::string_view divide = "\uE029";

constexpr std::string_view f1 = "\uE031";
constexpr std::string_view f2 = "\uE032";
constexpr std::string_view f3 = "\uE033";
constexpr std::string_view f4 = "\uE034";
constexpr std::string_view f5 = "\uE035";
constexpr std::string_view f6 = "\uE036";
constexpr std::string_view f7 = "\uE037";
constexpr std::string_view f8 = "\uE038";
constexpr std::string_view f9 = "\uE039";
constexpr std::string_view f10 = "\uE03A";
constexpr std::string_view f11 = "\uE03B";
constexpr std::string_view f12 = "\uE03C";

constexpr std::string_view command = "\uE03D";
constexpr std::string_view meta = "\uE03D";

} // namespace wdlite::key
