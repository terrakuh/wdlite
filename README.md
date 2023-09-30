# wdlite

Simple WebDriver header-only library for C++ 17. Uses Boost.ASIO or standalone ASIO with [cURLio](https://github.com/terrakuh/cURLio). This library follows the ASIO styling with `async_` functions and custom completion tokens. Not everything is implemented just the simple stuff. You are welcome to contribute more features.

## Simple usage

Here an example with a relatively recent ASIO library and C++ 20 coroutines.

```cpp
const auto session =
  co_await wdlite::async_new_session(co_await asio::this_coro::executor, "http://localhost:9515",
                                     wdlite::capabilities::make(), asio::use_awaitable);

co_await session->async_navigate("https://example.com", asio::use_awaitable);

std::cout << "Title: " << co_await session->async_get_title(asio::use_awaitable) << "\n";
```

Look at the examples for a better understanding.

## Dependencies

wdlite requires [nlohmann/json](https://github.com/nlohmann/json) (which can be automatically fetched from GitHub with FetchContent) and [cURLio](https://github.com/terrakuh/cURLio) which is currently a submodule. cURLio requires Boost and ASIO use `CURLIO_FETCH_DEPENDENCIES=ON` for automatically fetching those.
