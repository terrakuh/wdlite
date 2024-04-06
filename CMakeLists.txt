cmake_minimum_required(VERSION 3.18)

project(
  wdlite
  VERSION 0.1.0
  DESCRIPTION "Simple C++ WebDriver library"
  HOMEPAGE_URL "https://github.com/terrakuh/wdlite"
  LANGUAGES CXX
)

set(WDLITE_TOP_LEVEL ON)
get_directory_property(parent_directory PARENT_DIRECTORY)
if(parent_directory)
  set(WDLITE_TOP_LEVEL OFF)
endif()

option(WDLITE_BUILD_EXAMPLES "Build the provided examples." ${WDLITE_TOP_LEVEL})
option(WDLITE_ENABLE_LOGGING "Prints debug information. Mainly for development." OFF)
mark_as_advanced(WDLITE_ENABLE_LOGGING)

find_package(cURLio REQUIRED)
# add_subdirectory(vendor/cURLio)
find_package(nlohmann_json REQUIRED)

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
if(WDLITE_TOP_LEVEL_TOP_LEVEL)
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
