#pragma once

#include "session.hpp"

#include <optional>
#include <sstream>

namespace wdlite {

class Element {
public:
	using executor_type = Session::executor_type;

	executor_type get_executor() const noexcept;
	const std::string& get_id() const noexcept;

	template<typename Token>
	auto async_get_text(Token&& token);
	template<typename Token>
	auto async_get_attribute(std::string_view name, Token&& token);
	template<typename Token>
	auto async_get_property(std::string_view name, Token&& token);

	template<typename Token>
	auto async_click(Token&& token);
	template<typename Token>
	auto async_clear(Token&& token);
	template<typename Token>
	auto async_send_keys(std::string_view text, Token&& token);

	/// Takes a screenshot of the visible area of this element and returns a Base64 encoded string with the
	/// image data.
	template<typename Token>
	auto async_take_screenshot(Token&& token);

private:
	friend Session;

	std::shared_ptr<Session> _session;
	std::string _id;
	std::string _prefix;

	Element(std::shared_ptr<Session> session, std::string id);
};

template<typename... Values>
inline std::string make_keys(Values&&... values)
{
	std::stringstream stream{};
	(stream << ... << std::forward<Values>(values));
	return std::move(stream).str();
}

} // namespace wdlite
