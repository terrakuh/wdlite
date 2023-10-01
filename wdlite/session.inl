#include "element.hpp"
#include "session.hpp"

#include <type_traits>
#include <utility>
#include <variant>

namespace wdlite {

namespace detail {

template<typename Token, typename Lambda>
inline void complete_token(Token&& token, Lambda&& lambda, curlio::detail::asio_error_code ec,
                           nlohmann::json&& json)
{
	using type = decltype(lambda(ec, std::move(json)));
	if constexpr (std::is_void_v<type>) {
		if (!ec) {
			lambda(ec, std::move(json));
		}
		token.complete(ec);
	} else {
		if (ec) {
			token.complete(ec, type{});
		} else {
			auto value = lambda(ec, std::move(json));
			token.complete(ec, std::move(value));
		}
	}
}

constexpr std::string_view strategy_to_string(LocatorStrategy strategy) noexcept
{
	switch (strategy) {
	case LocatorStrategy::css_selector: return "css selector";
	case LocatorStrategy::link_text: return "link text";
	case LocatorStrategy::partial_link_text: return "partial link text";
	case LocatorStrategy::tag_name: return "tag name";
	case LocatorStrategy::xpath: return "xpath";
	}
	return "";
}

template<typename Lambda>
constexpr auto derive_asio_signature() noexcept
{
	using return_type = decltype(std::declval<Lambda>()(std::declval<curlio::detail::asio_error_code&>(),
	                                                    std::declval<nlohmann::json>()));
	if constexpr (std::is_void_v<return_type>) {
		return std::type_identity<void(curlio::detail::asio_error_code)>{};
	} else {
		return std::type_identity<void(curlio::detail::asio_error_code, return_type)>{};
	}
}

template<typename Lambda>
using AsioSignature = typename decltype(derive_asio_signature<Lambda>())::type;

} // namespace detail

inline Session::executor_type Session::get_executor() const noexcept { return _session->get_executor(); }

inline const std::string& Session::get_id() const noexcept { return _session_id; }

inline Session::Session(executor_type executor, std::string endpoint) : _endpoint{ std::move(endpoint) }
{
	_session = curlio::make_session(std::move(executor));

	if (!_endpoint.empty() && _endpoint.back() != '/') {
		_endpoint.push_back('/');
	}
}

template<typename Token>
inline auto Session::async_navigate(std::string_view url, Token&& token)
{
	return _post(_prefix + "/url", nlohmann::json{ { "url", url } }, std::forward<Token>(token),
	             [this](curlio::detail::asio_error_code& ec, nlohmann::json response) {
		             if (!ec) {
			             // TODO add checks
			             // session->_session_id = response["value"]["sessionId"].get<std::string>();
		             }
	             });
}

template<typename Token>
inline auto Session::async_get_current_url(Token&& token)
{
	return _get(_prefix + "/title", std::forward<Token>(token),
	            [](curlio::detail::asio_error_code& ec, nlohmann::json response) {
		            return response["value"].get<std::string>();
	            });
}

template<typename Token>
inline auto Session::async_get_title(Token&& token)
{
	return _get(_prefix + "/title", std::forward<Token>(token),
	            [](curlio::detail::asio_error_code& ec, nlohmann::json response) {
		            return response["value"].get<std::string>();
	            });
}

template<typename Token>
inline auto Session::async_get_page_source(Token&& token)
{
	return _get(_prefix + "/source", std::forward<Token>(token),
	            [](curlio::detail::asio_error_code& ec, nlohmann::json response) {
		            return std::move(response["value"].get_ref<std::string&>());
	            });
}

template<typename Token>
inline auto Session::async_find_element(std::string_view selector, LocatorStrategy strategy, Token&& token)
{
	return _post(_prefix + "/element",
	             nlohmann::json{ { "using", detail::strategy_to_string(strategy) }, { "value", selector } },
	             std::forward<Token>(token),
	             [this](curlio::detail::asio_error_code& ec, nlohmann::json response) {
		             return std::make_optional(Element{
		               shared_from_this(), std::move(response["value"].front().get_ref<std::string&>()) });
	             });
}

template<typename Token>
inline auto Session::async_find_elements(std::string_view selector, LocatorStrategy strategy, Token&& token)
{
	return _post(
	  _prefix + "/elements",
	  nlohmann::json{ { "using", detail::strategy_to_string(strategy) }, { "value", selector } },
	  std::forward<Token>(token), [this](curlio::detail::asio_error_code& ec, nlohmann::json response) {
		  return Element{ shared_from_this(), std::move(response["value"].front().get_ref<std::string&>()) };
	  });
}

template<typename Token, typename Lambda>
inline auto Session::_get(const std::string& endpoint, Token&& token, Lambda&& lambda)
{
	return _perform_request(endpoint, std::forward<Token>(token), std::forward<Lambda>(lambda),
	                        [](curlio::Request& /* request */) {});
}

template<typename Token, typename Lambda>
inline auto Session::_post(const std::string& endpoint, const nlohmann::json& payload, Token&& token,
                           Lambda&& lambda)
{
	return _perform_request(endpoint, std::forward<Token>(token), std::forward<Lambda>(lambda),
	                        [payload = payload.dump()](curlio::Request& request) {
		                        request.set_option<CURLOPT_COPYPOSTFIELDS>(payload.c_str());
		                        request.append_header("content-type: application/json");
		                        // std::cout << "sending: " << payload << "\n";
	                        });
}

template<typename Token, typename Lambda>
inline auto Session::_delete(const std::string& endpoint, const nlohmann::json& payload, Token&& token,
                             Lambda&& lambda)
{
	return _perform_request(endpoint, std::forward<Token>(token), std::forward<Lambda>(lambda),
	                        [payload = payload.dump()](curlio::Request& request) {
		                        request.set_option<CURLOPT_POSTFIELDS>(payload.c_str());
		                        request.set_option<CURLOPT_CUSTOMREQUEST>("DELETE");
		                        request.append_header("content-type: application/json");
	                        });
}

template<typename Token, typename Lambda, typename RequestModifier>
inline auto Session::_perform_request(const std::string& endpoint, Token&& token, Lambda&& lambda,
                                      RequestModifier&& modifier)
{
	auto request = curlio::make_request(_session);
	request->set_option<CURLOPT_URL>((_endpoint + endpoint).c_str());
	// request->set_option<CURLOPT_VERBOSE>(true);

	return CURLIO_ASIO_NS::async_compose<Token, detail::AsioSignature<Lambda>>(
	  [this, lambda = std::forward<Lambda>(lambda), modifier = std::forward<RequestModifier>(modifier),
	   request = std::move(request), response = curlio::Session::response_pointer{}](
	    auto& self, curlio::detail::asio_error_code ec = {},
	    std::variant<std::monostate, curlio::Session::response_pointer, std::string> result = {}) mutable {
		  // Any error is a bad error.
		  if (ec) {
			  detail::complete_token(self, lambda, ec, {});
			  return;
		  }

		  switch (result.index()) {
			// Initiate composition by starting the request.
		  case 0: {
			  modifier(*request);
			  _session->async_start(request, std::move(self));
			  break;
		  }
			// Request was started now read the response.
		  case 1: {
			  response = std::get<1>(std::move(result));
			  curlio::quick::async_read_all(response, std::move(self));
			  break;
		  }
			// Response was received now finish up.
		  case 2:
			  // std::cout << "result: " << std::get<2>(result) << "\n";
			  detail::complete_token(self, lambda, ec, nlohmann::json::parse(std::get<2>(result)));
			  break;
		  }
	  },
	  token, *this);
}

// Define this here to be able to call other functions.
inline Session::~Session()
{
	// Closing the last window will delete the session.
	_delete(_prefix + "/window", nlohmann::json::object(), CURLIO_ASIO_NS::use_future,
	        [](curlio::detail::asio_error_code& /* ec */, nlohmann::json /* response */) {});
}

template<typename Token>
inline auto async_new_session(Session::executor_type executor, std::string endpoint,
                              nlohmann::json capabilities, Token&& token)
{
	std::shared_ptr<Session> session{ new Session{ std::move(executor), std::move(endpoint) } };

	// The session for _post() is kept alive by the lambda passed.
	return session->_post("session", nlohmann::json{ { "capabilities", std::move(capabilities) } },
	                      std::forward<Token>(token),
	                      [session](curlio::detail::asio_error_code& ec, nlohmann::json response) mutable {
		                      if (!ec) {
			                      // TODO add checks
			                      session->_session_id = response["value"]["sessionId"].get<std::string>();
			                      session->_prefix = "session/" + session->_session_id;
		                      }
		                      return ec ? nullptr : std::move(session);
	                      });
}

} // namespace wdlite
