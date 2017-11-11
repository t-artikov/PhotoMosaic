cmake_minimum_required(VERSION 3.0)
project (PhotoMosaic)

set (CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD TRUE)
set(CMAKE_INSTALL_MESSAGE LAZY)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Qt)

file(GLOB SRC "src/*.cpp" "src/*.h")

set(TARGET PhotoMosaic)
add_executable(${TARGET} ${SRC})
target_link_libraries(${TARGET} Qt5::Core Qt5::Gui)
install(TARGETS ${TARGET} DESTINATION ".")
