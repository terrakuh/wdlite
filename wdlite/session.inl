#include "element.hpp"
#include "error.hpp"
#include "log.hpp"
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

inline std::size_t formatted_argument_list_size(const nlohmann::json& args) noexcept
{
	if (!args.is_object()) {
		return 0;
	}
	std::size_t size = 0;
	for (const auto& [key, _] : args.get_ref<const nlohmann::json::object_t&>()) {
		// +1 for the comma.
		if (size > 0) {
			++size;
		}
		size += key.size();
	}
	return size;
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

/**
 * Performs the WebDriver request for the given endpoint. This is just the generic implementation for
 * `_get()`, `_post()` and `_delete()`.
 *
 * @param session The cURLio session. The session will be kept alive as long as the request is running.
 * @param endpoint The full WebDriver URL.
 * @param token The ASIO completion token.
 * @param lambda This lambda will receive the JSON response from the WebDriver. The result of this lambda
 * will be forwarded to the completion token. The signature is `<return>(curlio::detail::asio_error_code&,
 * nlohmann::json)`.
 * @param modifier This modifier will be called once before the request starts. This instance will be kept
 * alive. The signature is `void(curlio::Request&)`.
 */
template<typename Token, typename Lambda, typename RequestModifier>
auto perform_request(std::shared_ptr<curlio::Session> session, const std::string& endpoint, Token&& token,
                     Lambda&& lambda, RequestModifier&& modifier)
{
	auto executor = session->get_executor();
	auto request = curlio::make_request(session);
	request->set_option<CURLOPT_URL>(endpoint.c_str());
	// request->set_option<CURLOPT_VERBOSE>(true);

	return CURLIO_ASIO_NS::async_compose<Token, detail::AsioSignature<Lambda>>(
	  [session = std::move(session), lambda = std::forward<Lambda>(lambda),
	   modifier = std::forward<RequestModifier>(modifier), request = std::move(request),
	   response = curlio::Session::response_pointer{}](
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
			  session->async_start(request, std::move(self));
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
			  WDLITE_DEBUG("Result: " << std::get<2>(result));
			  detail::complete_token(self, lambda, ec, nlohmann::json::parse(std::get<2>(result)));
			  break;
		  }
	  },
	  token, std::move(executor));
}

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
	return _post(
	  _prefix + "/element",
	  nlohmann::json{ { "using", detail::strategy_to_string(strategy) }, { "value", selector } },
	  std::forward<Token>(token),
	  [this](curlio::detail::asio_error_code& ec, nlohmann::json response) -> std::optional<Element> {
		  if (_check_error(ec, response)) {
			  return Element{ shared_from_this(), std::move(response["value"].front().get_ref<std::string&>()) };
		  } else if (ec == Code::no_such_element) {
			  ec = {};
		  }
		  return std::nullopt;
	  });
}

template<typename Token>
inline auto Session::async_find_elements(std::string_view selector, LocatorStrategy strategy, Token&& token)
{
	return _post(
	  _prefix + "/elements",
	  nlohmann::json{ { "using", detail::strategy_to_string(strategy) }, { "value", selector } },
	  std::forward<Token>(token), [this](curlio::detail::asio_error_code& ec, nlohmann::json response) {
		  std::vector<Element> elements{};
		  if (_check_error(ec, response)) {
			  for (auto& el : response["value"]) {
				  elements.push_back(Element{ shared_from_this(), std::move(el.front().get_ref<std::string&>()) });
			  }
		  }
		  return elements;
	  });
}

template<typename Token>
inline auto Session::async_execute_script_sync(std::string_view script, Token&& token)
{
	return _post(
	  _prefix + "/execute/sync", nlohmann::json{ { "script", script }, { "args", nlohmann::json::array() } },
	  std::forward<Token>(token), [this](curlio::detail::asio_error_code& ec, nlohmann::json response) {
		  return std::move(response["value"]);
	  });
}

template<typename Token>
inline auto Session::async_execute_script_async(std::string_view script, nlohmann::json arguments,
                                                Token&& token)
{
	nlohmann::json::array_t args = nlohmann::json::array();

	std::string tmp;
	tmp.reserve(script.size() + 16 + 2 + 81 + detail::formatted_argument_list_size(arguments));
	tmp = "(async function(";
	if (arguments.is_object()) {
		args.reserve(arguments.size());
		bool first = true;
		for (auto& [key, value] : arguments.get_ref<nlohmann::json::object_t&>()) {
			if (!first) {
				tmp += ',';
			}
			first = false;
			tmp += key;
			args.push_back(std::move(value));
		}
	} else if (!arguments.is_null()) {
		throw std::runtime_error{ "arguments must be null or an object" };
	}
	tmp += "){";
	tmp += script;
	tmp += "})().catch(arguments[arguments.length - 1]).then(arguments[arguments.length - 1])";

	return _post(
	  _prefix + "/execute/async", nlohmann::json{ { "script", std::move(tmp) }, { "args", std::move(args) } },
	  std::forward<Token>(token), [this](curlio::detail::asio_error_code& /* ec */, nlohmann::json response) {
		  return std::move(response["value"]);
	  });
}

template<typename Token, typename Lambda>
inline auto Session::_get(const std::string& endpoint, Token&& token, Lambda&& lambda)
{
	return detail::perform_request(_session, _endpoint + endpoint, std::forward<Token>(token),
	                               std::forward<Lambda>(lambda), [](curlio::Request& /* request */) {});
}

template<typename Token, typename Lambda>
inline auto Session::_post(const std::string& endpoint, const nlohmann::json& payload, Token&& token,
                           Lambda&& lambda)
{
	return detail::perform_request(_session, _endpoint + endpoint, std::forward<Token>(token),
	                               std::forward<Lambda>(lambda),
	                               [payload = payload.dump()](curlio::Request& request) {
		                               request.set_option<CURLOPT_COPYPOSTFIELDS>(payload.c_str());
		                               request.append_header("content-type: application/json");
		                               WDLITE_DEBUG("Sending: " << payload);
	                               });
}

template<typename Token, typename Lambda>
inline auto Session::_delete(const std::string& endpoint, const nlohmann::json& payload, Token&& token,
                             Lambda&& lambda)
{
	return detail::perform_request(_session, _endpoint + endpoint, std::forward<Token>(token),
	                               std::forward<Lambda>(lambda),
	                               [payload = payload.dump()](curlio::Request& request) {
		                               request.set_option<CURLOPT_POSTFIELDS>(payload.c_str());
		                               request.set_option<CURLOPT_CUSTOMREQUEST>("DELETE");
		                               request.append_header("content-type: application/json");
	                               });
}

inline bool Session::_check_error(curlio::detail::asio_error_code& ec, const nlohmann::json& response)
{
	if (ec) {
		return false;
	} else if (const auto vit = response.find("value");
	           vit == response.end() || !(vit->is_object() || vit->is_array())) {
		ec = Code::unknown_webdirver_error;
	} else if (vit->is_array()) {
		return true;
	} else if (const auto eit = vit->find("error"); eit != vit->end() && eit->is_string()) {
		ec = convert_webdriver_error(eit->get_ref<const std::string&>());
	} else {
		return true;
	}
	return false;
}

// Define this here to be able to call other functions.
inline Session::~Session()
{
	// Closing the last window will delete the session. It is safe to call this function in the destructor as
	// the internal `detail::perform_request()` does not rely on this instance.
	_delete(_prefix + "/window", nlohmann::json::object(), CURLIO_ASIO_NS::detached,
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
