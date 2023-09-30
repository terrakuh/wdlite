add_executable(wdlite-simple simple.cpp)
target_link_libraries(wdlite-simple PRIVATE wdlite::wdlite)
target_compile_features(wdlite-simple PRIVATE cxx_std_20)

add_executable(wdlite-playground playground.cpp)
target_link_libraries(wdlite-playground PRIVATE wdlite::wdlite)
target_compile_features(wdlite-playground PRIVATE cxx_std_20)
