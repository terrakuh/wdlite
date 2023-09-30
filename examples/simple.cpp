#include <iostream>
#include <wdlite/wdlite.hpp>

namespace asio = CURLIO_ASIO_NS;

inline asio::awaitable<std::string> get_title(std::string_view url)
{
	const auto session =
	  co_await wdlite::async_new_session(co_await asio::this_coro::executor, "http://localhost:9515",
	                                     wdlite::capabilities::make(wdlite::capabilities::Capabilities{
	                                       .browser_specific =
	                                         wdlite::capabilities::ChromeOptions{
	                                           .arguments = { "--headless=new" },
	                                         },
	                                     }),
	                                     asio::use_awaitable);

	co_await session->async_navigate(url, asio::use_awaitable);

	co_return co_await session->async_get_title(asio::use_awaitable);
}

int main(int argc, char** argv)
{
	if (argc <= 1) {
		std::cerr << "You need to provide an URL\n";
		return 1;
	}

	asio::io_service service{};

	asio::co_spawn(
	  service,
	  [&]() -> asio::awaitable<void> {
		  const auto title = co_await get_title(argv[1]);
		  std::cout << "The title of '" << argv[1] << "' is: " << title << "\n";

		  co_return;
	  }(),
	  asio::detached);

	service.run();
	return 0;
}
