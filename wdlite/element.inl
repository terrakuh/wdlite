#include "element.hpp"

namespace wdlite {

inline Element::executor_type Element::get_executor() const noexcept { return _session->get_executor(); }

inline const std::string& Element::get_id() const noexcept { return _id; }

template<typename Token>
inline auto Element::async_get_text(Token&& token) const
{
	return _session->_get(_prefix + "/text", std::forward<Token>(token),
	                      [](curlio::detail::asio_error_code& /* ec */, nlohmann::json response) {
		                      return std::move(response["value"].get_ref<std::string&>());
	                      });
}

template<typename Token>
inline auto Element::async_get_attribute(std::string_view name, Token&& token) const
{
	return _session->_get(make_keys(_prefix, "/attribute/", name), std::forward<Token>(token),
	                      [](curlio::detail::asio_error_code& /* ec */, nlohmann::json response) {
		                      const auto it = response.find("value");
		                      return it == response.end()
		                               ? std::nullopt
		                               : std::make_optional(std::move(it->get_ref<std::string&>()));
	                      });
}

template<typename Token>
inline auto Element::async_get_property(std::string_view name, Token&& token) const
{
	return _session->_get(make_keys(_prefix, "/property/", name), std::forward<Token>(token),
	                      [](curlio::detail::asio_error_code& /* ec */, nlohmann::json response) {
		                      const auto it = response.find("value");
		                      return it == response.end()
		                               ? std::nullopt
		                               : std::make_optional(std::move(it->get_ref<std::string&>()));
	                      });
}

template<typename Token>
inline auto Element::async_find_element(std::string_view selector, LocatorStrategy strategy,
                                        Token&& token) const
{
	return _session->_async_find_element(_prefix + "/element", selector, strategy, std::forward<Token>(token));
}

template<typename Token>
inline auto Element::async_find_elements(std::string_view selector, LocatorStrategy strategy,
                                         Token&& token) const
{
	return _session->_async_find_elements(_prefix + "/elements", selector, strategy,
	                                      std::forward<Token>(token));
}

template<typename Token>
inline auto Element::async_click(Token&& token)
{
	return _session->_post(_prefix + "/click", nlohmann::json::object(), std::forward<Token>(token),
	                       [](curlio::detail::asio_error_code& /* ec */, nlohmann::json /* response */) {});
}

template<typename Token>
inline auto Element::async_clear(Token&& token)
{
	return _session->_post(_prefix + "/clear", nlohmann::json::object(), std::forward<Token>(token),
	                       [](curlio::detail::asio_error_code& /* ec */, nlohmann::json /* response */) {});
}

template<typename Token>
inline auto Element::async_send_keys(std::string_view text, Token&& token)
{
	return _session->_post(_prefix + "/value", nlohmann::json{ { "text", text } }, std::forward<Token>(token),
	                       [](curlio::detail::asio_error_code& /* ec */, nlohmann::json /* response */) {});
}

template<typename Token>
inline auto Element::async_take_screenshot(Token&& token) const
{
	return _session->_get(_prefix + "/screenshot", std::forward<Token>(token),
	                      [](curlio::detail::asio_error_code& /* ec */, nlohmann::json response) {
		                      return std::move(response["value"].get_ref<std::string&>());
	                      });
}

inline Element::Element(std::shared_ptr<Session> session, std::string id)
    : _session{ std::move(session) }, _id{ std::move(id) }
{
	_prefix = "session/" + _session->_session_id + "/element/" + _id;
}

} // namespace wdlite
