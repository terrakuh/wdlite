#include <iostream>
#include <wdlite/wdlite.hpp>

namespace asio = CURLIO_ASIO_NS;

int main()
{
	asio::io_service service{};

	asio::co_spawn(
	  service,
	  [&]() -> asio::awaitable<void> {
		  auto session = co_await wdlite::async_new_session(
		    service.get_executor(), "http://localhost:9515",
		    wdlite::capabilities::make(wdlite::capabilities::Capabilities{
		      .browser_specific =
		        wdlite::capabilities::ChromeOptions{
		          .arguments = { "--headless=new", "--user-data-dir=/path/to/your/custom/profile" },
		        },
		    }),
		    asio::use_awaitable);

		  std::cout << "Session created with ID=" << session->get_id() << "\n";

		  co_await session->async_navigate("https://example.com", asio::use_awaitable);
		  // std::cout << "Title: " << (co_await session->async_get_title(asio::use_awaitable)) << "\n";
		  // std::cout << "Title: " << (co_await session->async_get_page_source(asio::use_awaitable)) << "\n";

		  auto open = co_await session->async_find_element("#loginNav", wdlite::LocatorStrategy::css_selector,
		                                                   asio::use_awaitable);

		  // co_await open.async_click(asio::use_awaitable);

		  // auto username = co_await session->async_find_element(
		  //   "#mod_login_username", wdlite::LocatorStrategy::css_selector, asio::use_awaitable);
		  // co_await username.async_send_keys("username", asio::use_awaitable);
		  // std::cout << "text is: " << (co_await username.async_get_property("value",
		  // asio::use_awaitable)).value()
		  //           << "\n";
		  // co_await username.async_clear(asio::use_awaitable);

		  co_return;
	  }(),
	  asio::detached);

	service.run();
}
