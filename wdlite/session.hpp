#pragma once

#include "fwd.hpp"

#include <curlio/curlio.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace wdlite {

enum class LocatorStrategy {
	css_selector,
	link_text,
	partial_link_text,
	tag_name,
	xpath,
};

class Session : public std::enable_shared_from_this<Session> {
public:
	using executor_type = curlio::Session::executor_type;

	/// When the session instance is destroyed. The remote window is closed.
	~Session();

	/**
	 * Creates a new session object and opens the session on the remote WebDriver endpoint.
	 *
	 * @param executor The ASIO executor for the HTTP request and asynchronous actions.
	 * @param endpoint The WebDriver endpoint URL. For example: `http://localhost:9515`.
	 * @param capabilities Desired capabilities sent to the WebDriver. This object can be custom or created by
	 * `wdlite::capabilities::make()`. See [here](https://w3c.github.io/webdriver/#capabilities) for more
	 * information.
	 * @param token The ASIO completion token.
	 * @return A newly created session as `std::shared_ptr<Session>`.
	 */
	template<typename Token>
	friend auto async_new_session(executor_type executor, std::string endpoint, nlohmann::json capabilities,
	                              Token&& token);

	executor_type get_executor() const noexcept;
	/// The WebDriver session ID.
	const std::string& get_id() const noexcept;

	/**
	 * Instructs the browser to navigate to the given URL.
	 *
	 * @param url The fully qualified URL.
	 * @param token The ASIO completion token.
	 */
	template<typename Token>
	auto async_navigate(std::string_view url, Token&& token);
	/**
	 * Retrieves the current URL.
	 *
	 * @param token The ASIO completion token.
	 * @return The result stored in a `std::string` depending on `token`.
	 */
	template<typename Token>
	auto async_get_current_url(Token&& token);
	/**
	 * Retrieves the current pages title.
	 *
	 * @param token The ASIO completion token.
	 * @return The result stored in a `std::string` depending on `token`.
	 */
	template<typename Token>
	auto async_get_title(Token&& token);
	/**
	 * Retrieves the current pages source code.
	 *
	 * @param token The ASIO completion token.
	 * @return The result stored in a `std::string` depending on `token`.
	 */
	template<typename Token>
	auto async_get_page_source(Token&& token);

	template<typename Token>
	auto async_find_element(std::string_view selector, LocatorStrategy strategy, Token&& token);
	template<typename Token>
	auto async_find_elements(std::string_view selector, LocatorStrategy strategy, Token&& token);

private:
	friend Element;

	std::shared_ptr<curlio::Session> _session;
	std::string _endpoint;
	std::string _session_id;
	/// A precomputed prefix string for the session endpoints.
	std::string _prefix;

	/// Just instantiates the object but does not create the remote session.
	Session(executor_type executor, std::string endpoint);

	template<typename Token, typename Lambda>
	auto _get(const std::string& endpoint, Token&& token, Lambda&& lambda);
	template<typename Token, typename Lambda>
	auto _post(const std::string& endpoint, const nlohmann::json& payload, Token&& token, Lambda&& lambda);
	template<typename Token, typename Lambda>
	auto _delete(const std::string& endpoint, const nlohmann::json& payload, Token&& token, Lambda&& lambda);
	/**
	 * Performs the WebDriver request for the given endpoint. This is just the generic implementation for
	 * `_get()`, `_post()` and `_delete()`.
	 *
	 * @param endpoint Joined together with first part of the WebDriver URL.
	 * @param token The ASIO completion token.
	 * @param lambda This lambda will receive the JSON response from the WebDriver. The result of this lambda
	 * will be forwarded to the completion token. The signature is `<return>(curlio::detail::asio_error_code&,
	 * nlohmann::json)`.
	 * @param modifier This modifier will be called once before the request starts. This instance will be kept
	 * alive. The signature is `void(curlio::Request&)`.
	 */
	template<typename Token, typename Lambda, typename RequestModifier>
	auto _perform_request(const std::string& endpoint, Token&& token, Lambda&& lambda,
	                      RequestModifier&& modifier);
};

} // namespace wdlite
