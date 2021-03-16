cmake_minimum_required(VERSION 3.10)
include(ExternalProject)

project(libonchain)

execute_process(COMMAND git submodule update --init --recursive WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# note: the Fetch commands are maintained better than ExternalProject and accept the same behavior
ExternalProject_add(
    libbtc_submodule SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/libbtc
    PATCH_COMMAND ./autogen.sh CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make INSTALL_COMMAND make install #TEST_COMMAND make check
)
ExternalProject_Get_Property(libbtc_submodule INSTALL_DIR)
# note: this also makes a pkgconfig file that could be used.  for some reason the btc includes are not installed in a btc subfolder; they really look intended to be.
add_library(libbtc INTERFACE)
add_dependencies(libbtc libbtc_submodule)
target_link_libraries(libbtc INTERFACE ${INSTALL_DIR}/lib/libbtc.a)
target_include_directories(libbtc INTERFACE ${INSTALL_DIR}/include)

add_library(libonchain src/chain_libbtc.cpp src/chain_libbtc_bsv.cpp)
target_link_libraries(libonchain libbtc)
target_include_directories(libonchain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(onchain src/tools/onchain.cpp)
target_link_libraries(onchain libonchain)
