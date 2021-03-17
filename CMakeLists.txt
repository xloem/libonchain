cmake_minimum_required(VERSION 3.13)
include(ExternalProject)

project(libonchain)

execute_process(COMMAND git submodule update --init --recursive WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads)
find_package(PkgConfig)

pkg_check_modules(event REQUIRED IMPORTED_TARGET libevent)
find_library(gmp gmp REQUIRED)
# note: the Fetch commands are maintained better than ExternalProject and accept the same behavior
ExternalProject_add(
    libbtc_submodule SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/libbtc
    PATCH_COMMAND ./autogen.sh CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> #CFLAGS=-fPIC
    BUILD_COMMAND make INSTALL_COMMAND make install #TEST_COMMAND make check
)
ExternalProject_Get_Property(libbtc_submodule INSTALL_DIR)
ExternalProject_Get_Property(libbtc_submodule BINARY_DIR)
# to use pkgconfig:
#list(APPEND CMAKE_PREFIX_PATH ${INSTALL_DIR})
#pkg_check_modules(libbtc REQUIRED IMPORTED_TARGET libbtc)
## one option with libsecp256k1 is to build and install the libbtc in-tree version, then configure libbtc to use it, but libbtc should really provide for a working shared library installation
##pkg_check_modules(libsecp256k1 REQUIRED IMPORTED_TARGET libsecp256k1)

add_library(libbtc INTERFACE)
add_dependencies(libbtc libbtc_submodule)
target_link_libraries(libbtc INTERFACE ${INSTALL_DIR}/lib/libbtc.a ${BINARY_DIR}/src/secp256k1/.libs/libsecp256k1.a PkgConfig::event gmp)# PkgConfig::libsecp256k1
##target_link_libraries(libbtc INTERFACE libbtc libsecp256k1)
target_include_directories(libbtc INTERFACE ${INSTALL_DIR}/include) # ${SECP256K1_INCLUDE_DIRS})

add_library(onchain src/chain.cpp src/chain_libbtc.cpp)
target_link_libraries(onchain PRIVATE Threads::Threads libbtc)# PkgConfig::libsecp256k1 PkgConfig::libbtc)
target_include_directories(onchain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${INSTALL_DIR}/include)

add_executable(onchain-bin src/tools/onchain.cpp)
target_link_libraries(onchain-bin onchain)
set_target_properties(onchain-bin PROPERTIES OUTPUT_NAME onchain)
