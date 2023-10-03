cmake_minimum_required(VERSION 3.18)

project(
  wdlite
  VERSION 0.1.0
  DESCRIPTION "Simple C++ WebDriver library"
  HOMEPAGE_URL "https://github.com/terrakuh/wdlite"
  LANGUAGES CXX
)

set(${PROJECT_NAME}_TOP_LEVEL ON)
get_directory_property(parent_directory PARENT_DIRECTORY)
if(parent_directory)
  set(${PROJECT_NAME}_TOP_LEVEL OFF)
endif()

option(WDLITE_BUILD_EXAMPLES "Build the provided examples." ${PROJECT_NAME}_TOP_LEVEL)
option(WDLITE_FETCH_DEPENDENCIES "Fetches the required dependencies automatically." OFF)
option(WDLITE_ENABLE_LOGGING "Prints debug information. Mainly for development." OFF)
mark_as_advanced(WDLITE_ENABLE_LOGGING)

set(required REQUIRED)
if(WDLITE_FETCH_DEPENDENCIES)
  set(required QUIET)
endif()

find_package(nlohmann_json ${required})
if(NOT nlohmann_json_FOUND AND WDLITE_FETCH_DEPENDENCIES)
  message(STATUS "nlohmann_json not found. Fetching from GitHub.")
  include(FetchContent)
  set(JSON_Install ON)
  FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.2.zip
    URL_HASH SHA256=95651d7d1fcf2e5c3163c3d37df6d6b3e9e5027299e6bd050d157322ceda9ac9
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()

add_subdirectory(vendor/cURLio)

add_subdirectory(wdlite)

if(WDLITE_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Install
include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${PROJECT_NAME}-config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  INSTALL_DESTINATION "${LIBRARY_INSTALL_DIR}/cmake/${PROJECT_NAME}"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        DESTINATION lib/cmake/${PROJECT_NAME}
)
install(
  EXPORT ${PROJECT_NAME}-targets
  DESTINATION lib/cmake/${PROJECT_NAME}
  NAMESPACE ${PROJECT_NAME}::
  EXPORT_LINK_INTERFACE_LIBRARIES
)

# CPack
if(${PROJECT_NAME}_TOP_LEVEL)
  set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
  set(CPACK_PACKAGE_CONTACT "Yunus Ayar")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}")
  set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
  set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
  set(CPACK_GENERATOR DEB TGZ)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

  include(CPack)
endif()
