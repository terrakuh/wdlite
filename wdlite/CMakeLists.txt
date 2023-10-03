file(GLOB_RECURSE sources "${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]pp")

add_library(wdlite INTERFACE ${sources})
add_library(wdlite::wdlite ALIAS wdlite)

target_include_directories(
  wdlite INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>" $<INSTALL_INTERFACE:include>
)
target_link_libraries(wdlite INTERFACE curlio::curlio nlohmann_json::nlohmann_json)
target_compile_features(wdlite INTERFACE cxx_std_17)

if(WDLITE_ENABLE_LOGGING)
  target_compile_definitions(wdlite INTERFACE WDLITE_ENABLE_LOGGING)
endif()

install(TARGETS wdlite EXPORT ${PROJECT_NAME}-targets)
install(
  DIRECTORY .
  DESTINATION include/${PROJECT_NAME}
  FILES_MATCHING
  PATTERN "*.hpp"
  PATTERN "*.inl"
)
