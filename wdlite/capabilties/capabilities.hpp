#pragma once

#include "chrome.hpp"

#include <variant>

namespace wdlite::capabilities {

/// The options are the general capabilities for the WebDriver. The descriptions were copied from the official
/// [documentation](https://w3c.github.io/webdriver/#capabilities).
struct Capabilities {
	/// Identifies the user agent.
	std::string browser_name;
	/// Identifies the version of the user agent.
	std::string browser_version;

	std::variant<std::monostate, ChromeOptions> browser_specific;
};

inline void to_json(nlohmann::json& json, const Capabilities& value)
{
	json = nlohmann::json::object();

	const auto set = [&](const char* key, const auto& value) {
		bool do_set = false;
		if constexpr (std::is_same_v<std::decay_t<decltype(value)>, nlohmann::json>) {
			do_set = !value.is_null();
		} else {
			do_set = !value.empty();
		}
		if (do_set) {
			json[key] = value;
		}
	};

	constexpr std::string_view browser_names[] = { "", "chrome" };
	set("browserName", value.browser_name.empty() ? browser_names[value.browser_specific.index()]
	                                              : std::string_view{ value.browser_name });
	set("browserVersion", value.browser_version);

	std::visit(
	  [&](const auto& specific) {
		  if constexpr (!std::is_same_v<std::decay_t<decltype(specific)>, std::monostate>) {
			  constexpr std::string_view option_names[] = { "goog:chromeOptions" };
			  json[option_names[value.browser_specific.index() - 1]] = specific;
		  }
	  },
	  value.browser_specific);
}

template<typename... FirstMatches>
inline nlohmann::json make(const nlohmann::json& always_match, FirstMatches&&... first_matches)
{
	nlohmann::json json = nlohmann::json::object();
	if (!always_match.is_null()) {
		json["alwaysMatch"] = always_match;
	}
	if constexpr (sizeof...(FirstMatches) > 0) {
		auto array = nlohmann::json::array();
		(array.push_back(std::forward<FirstMatches>(first_matches)), ...);
		json["firstMatches"] = std::move(array);
	}
	return json;
}

template<typename... FirstMatches>
inline nlohmann::json make(nlohmann::json&& always_match = {}, FirstMatches&&... first_matches)
{
	nlohmann::json json = nlohmann::json::object();
	if (!always_match.is_null()) {
		json["alwaysMatch"] = std::move(always_match);
	}
	if constexpr (sizeof...(FirstMatches) > 0) {
		auto array = nlohmann::json::array();
		(array.push_back(std::forward<FirstMatches>(first_matches)), ...);
		json["firstMatches"] = std::move(array);
	}
	return json;
}

} // namespace wdlite::capabilities
