#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace wdlite::capabilities {

/// The options specifically for the ChromeWebdriver. The descriptions were copied from the official
/// documentation (https://chromedriver.chromium.org/capabilities).
struct ChromeOptions {
	struct PerformanceLoggingPreferences {
		/// Whether or not to collect events from Network domain.
		bool enable_network = true;
		/// Whether or not to collect events from Page domain.
		bool enable_page = true;
		/// A comma-separated string of Chrome tracing categories for which trace events should be collected. An
		/// unspecified or empty string disables tracing.
		std::string trace_categories;
		/// The requested number of milliseconds between DevTools trace buffer usage events. For example, if 1000,
		/// then once per second, DevTools will report how full the trace buffer is. If a report indicates the
		/// buffer usage is 100%, a warning will be issued.
		unsigned int buffer_usage_reporting_interval = 1000;
	};

	/// List of command-line arguments to use when starting Chrome. Arguments with an associated value should be
	/// separated by a '=' sign (e.g., ['start-maximized', 'user-data-dir=/tmp/temp_profile']). See
	/// [here](http://peter.sh/experiments/chromium-command-line-switches/) for a list of Chrome arguments.
	std::vector<std::string> arguments;
	/// Path to the Chrome executable to use (on Mac OS X, this should be the actual binary, not just the app.
	/// e.g., '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome')
	std::string binary;
	/// A list of Chrome extensions to install on startup. Each item in the list should be a base-64 encoded
	/// packed Chrome extension (.crx)
	std::vector<std::string> extensions;
	/// A dictionary with each entry consisting of the name of the preference and its value. These preferences
	/// are applied to the Local State file in the user data folder.
	nlohmann::json local_state;
	/// A dictionary with each entry consisting of the name of the preference and its value. These preferences
	/// are only applied to the user profile in use. See the 'Preferences' file in Chrome's user data directory
	/// for examples.
	nlohmann::json preferences;
	/// If false, Chrome will be quit when ChromeDriver is killed, regardless of whether the session is quit. If
	/// true, Chrome will only be quit if the session is quit (or closed). Note, if true, and the session is not
	/// quit, ChromeDriver cannot clean up the temporary user data directory that the running Chrome instance is
	/// using.
	bool detach = false;
	/// An address of a Chrome debugger server to connect to, in the form of <hostname/ip:port>, e.g.
	/// '127.0.0.1:38947'
	std::string debugger_address;
	/// List of Chrome command line switches to exclude that ChromeDriver by default passes when starting
	/// Chrome.  Do not prefix switches with --.
	std::vector<std::string> exclude_switches;
	/// Directory to store Chrome minidumps . (Supported only on Linux.)
	std::string minidump_path;
	/// A dictionary with either a value for “deviceName,” or values for “deviceMetrics” and “userAgent.” Refer
	/// to Mobile Emulation for more information.
	nlohmann::json mobile_emulation;
	/// An optional dictionary that specifies performance logging preferences.
	std::optional<PerformanceLoggingPreferences> performance_logging_preferences;
	/// A list of window types that will appear in the list of window handles. For access to <webview> elements,
	/// include "webview" in this list.
	std::vector<std::string> window_types;
};

inline void to_json(nlohmann::json& json, const ChromeOptions::PerformanceLoggingPreferences& value)
{
	json = nlohmann::json{
		{ "enableNetwork", value.enable_network },
		{ "enablePage", value.enable_page },
		{ "traceCategories", value.trace_categories },
		{ "bufferUsageReportingInterval", value.buffer_usage_reporting_interval },
	};
}

inline void to_json(nlohmann::json& json, const ChromeOptions& value)
{
	json = nlohmann::json{ { "detach", value.detach } };

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

	set("args", value.arguments);
	set("binary", value.binary);
	set("extensions", value.extensions);
	set("localState", value.local_state);
	set("prefs", value.preferences);
	set("debuggerAddress", value.debugger_address);
	set("excludeSwitches", value.exclude_switches);
	set("minidumpPath", value.minidump_path);
	set("mobileEmulation", value.mobile_emulation);
	if (value.performance_logging_preferences.has_value()) {
		json["perfLoggingPrefs"] = value.performance_logging_preferences.value();
	}
	set("windowTypes", value.window_types);
}

} // namespace wdlite::capabilities
