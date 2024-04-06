#pragma once

#include <map>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace wdlite {

enum class Code {
	success,

	unknown_webdirver_error,
	element_click_intercepted,
	element_not_interactable,
	insecure_certificate,
	invalid_argument,
	invalid_cookie_domain,
	invalid_element_state,
	invalid_selector,
	invalid_session_id,
	javascript_error,
	move_target_out_of_bounds,
	no_such_alert,
	no_such_cookie,
	no_such_element,
	no_such_frame,
	no_such_window,
	no_such_shadow_root,
	script_timeout,
	session_not_created,
	stale_element_reference,
	detached_shadow_root,
	timeout,
	unable_to_set_cookie,
	unable_to_capture_screen,
	unexpected_alert_open,
	unknown_command,
	unknown_error,
	unknown_method,
	unsupported_operation,
};

enum class Condition {
	success,
	webdriver_error,
};

[[nodiscard]] inline Code convert_webdriver_error(std::string_view error) noexcept
{
	static const std::map<std::string_view, Code> map{
		{ "element click intercepted", Code::element_click_intercepted },
		{ "element not interactable", Code::element_not_interactable },
		{ "insecure certificate", Code::insecure_certificate },
		{ "invalid argument", Code::invalid_argument },
		{ "invalid cookie domain", Code::invalid_cookie_domain },
		{ "invalid element state", Code::invalid_element_state },
		{ "invalid selector", Code::invalid_selector },
		{ "invalid session id", Code::invalid_session_id },
		{ "javascript error", Code::javascript_error },
		{ "move target out of bounds", Code::move_target_out_of_bounds },
		{ "no such alert", Code::no_such_alert },
		{ "no such cookie", Code::no_such_cookie },
		{ "no such element", Code::no_such_element },
		{ "no such frame", Code::no_such_frame },
		{ "no such window", Code::no_such_window },
		{ "no such shadow root", Code::no_such_shadow_root },
		{ "script timeout", Code::script_timeout },
		{ "session not created", Code::session_not_created },
		{ "stale element reference", Code::stale_element_reference },
		{ "detached shadow root", Code::detached_shadow_root },
		{ "timeout", Code::timeout },
		{ "unable to set cookie", Code::unable_to_set_cookie },
		{ "unable to capture screen", Code::unable_to_capture_screen },
		{ "unexpected alert open", Code::unexpected_alert_open },
		{ "unknown command", Code::unknown_command },
		{ "unknown error", Code::unknown_error },
		{ "unknown method", Code::unknown_method },
		{ "unsupported operation", Code::unsupported_operation },
	};

	if (const auto it = map.find(error); it != map.end()) {
		return it->second;
	}
	return Code::unknown_webdirver_error;
}

[[nodiscard]] std::error_condition make_error_condition(Condition condition) noexcept;

[[nodiscard]] inline const std::error_category& code_category() noexcept
{
	static class : public std::error_category {
	public:
		const char* name() const noexcept override { return "wdlite"; }
		std::error_condition default_error_condition(int code) const noexcept override
		{
			if (code == 0) {
				return make_error_condition(Condition::success);
			} else if (code >= 1 && code < 100) {
				return make_error_condition(Condition::webdriver_error);
			}
			return error_category::default_error_condition(code);
		}
		std::string message(int ec) const override
		{
			switch (static_cast<Code>(ec)) {
			case Code::success: return "success";

			case Code::unknown_webdirver_error: return "unknown webdriver error";
			// The following error code are copied from the W3C reference
			// (https://www.w3.org/TR/webdriver2/#errors).
			case Code::element_click_intercepted:
				return "The Element Click command could not be completed because the element receiving the events is "
				       "obscuring the element that was requested clicked.";
			case Code::element_not_interactable:
				return "A command could not be completed because the element is not pointer- or keyboard "
				       "interactable.";
			case Code::insecure_certificate:
				return "Navigation caused the user agent to hit a certificate warning, which is usually the result "
				       "of an expired or invalid TLS certificate.";
			case Code::invalid_argument:
				return "The arguments passed to a command are either invalid or malformed.";
			case Code::invalid_cookie_domain:
				return "An illegal attempt was made to set a cookie under a different domain than the current page.";
			case Code::invalid_element_state:
				return "A command could not be completed because the element is in an invalid state, e.g. attempting "
				       "to clear an element that isn't both editable and resettable.";
			case Code::invalid_selector: return "Argument was an invalid selector.";
			case Code::invalid_session_id:
				return "Occurs if the given session id is not in the list of active sessions, meaning the session "
				       "either does not exist or that it's not active.";
			case Code::javascript_error:
				return "An error occurred while executing JavaScript supplied by the user.";
			case Code::move_target_out_of_bounds:
				return "The target for mouse interaction is not in the browser's viewport and cannot be brought into "
				       "that viewport.";
			case Code::no_such_alert:
				return "An attempt was made to operate on a modal dialog when one was not open.";
			case Code::no_such_cookie:
				return "No cookie matching the given path name was found amongst the associated cookies of session's "
				       "current browsing context's active document.";
			case Code::no_such_element:
				return "An element could not be located on the page using the given search parameters.";
			case Code::no_such_frame:
				return "A command to switch to a frame could not be satisfied because the frame could not be found.";
			case Code::no_such_window:
				return "A command to switch to a window could not be satisfied because the window could not be "
				       "found.";
			case Code::no_such_shadow_root: return "The element does not have a shadow root.";
			case Code::script_timeout: return "A script did not complete before its timeout expired.";
			case Code::session_not_created: return "A new session could not be created.";
			case Code::stale_element_reference:
				return "A command failed because the referenced element is no longer attached to the DOM.";
			case Code::detached_shadow_root:
				return "A command failed because the referenced shadow root is no longer attached to the DOM.";
			case Code::timeout: return "An operation did not complete before its timeout expired.";
			case Code::unable_to_set_cookie: return "A command to set a cookie's value could not be satisfied.";
			case Code::unable_to_capture_screen: return "A screen capture was made impossible.";
			case Code::unexpected_alert_open: return "A modal dialog was open, blocking this operation.";
			case Code::unknown_command:
				return "A command could not be executed because the remote end is not aware of it.";
			case Code::unknown_error:
				return "An unknown error occurred in the remote end while processing the command.";
			case Code::unknown_method:
				return "The requested command matched a known URL but did not match any method for that URL.";
			case Code::unsupported_operation:
				return "Indicates that a command that should have executed properly cannot be supported for some "
				       "reason.";

			default: return "(unrecognized error code)";
			}
		}
	} category;
	return category;
}

[[nodiscard]] inline const std::error_category& condition_category() noexcept
{
	static class : public std::error_category {
	public:
		const char* name() const noexcept override { return "wdlite"; }
		std::string message(int condition) const override
		{
			switch (static_cast<Condition>(condition)) {
			case Condition::success: return "success";
			case Condition::webdriver_error: return "webdriver";
			default: return "(unrecognized error condition)";
			}
		}
	} category;
	return category;
}

[[nodiscard]] inline std::error_code make_error_code(Code code) noexcept
{
	return { static_cast<int>(code), code_category() };
}

[[nodiscard]] inline std::error_condition make_error_condition(Condition condition) noexcept
{
	return { static_cast<int>(condition), condition_category() };
}

} // namespace wdlite

namespace std {

template<>
struct is_error_code_enum<wdlite::Code> : true_type {};

template<>
struct is_error_condition_enum<wdlite::Condition> : true_type {};

} // namespace std
