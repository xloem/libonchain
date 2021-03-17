cmake_minimum_required(VERSION 3.11)

project(libonchain)

find_package(Threads)
find_package(PkgConfig)
include(ExternalProject)

execute_process(COMMAND git submodule update --init --recursive WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(libbtc INTERFACE)
    ExternalProject_add(
        libbtc_submodule SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/libbtc
        PATCH_COMMAND ./autogen.sh CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
        BUILD_COMMAND make INSTALL_COMMAND make install #TEST_COMMAND make check
    )
    ExternalProject_Get_Property(libbtc_submodule INSTALL_DIR)
    link_directories(libbtc INTERFACE ${INSTALL_DIR}/lib)
    include_directories(libbtc INTERFACE ${INSTALL_DIR}/include)
    
    ExternalProject_add(
        secp256k1_submodule SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/secp256k1
        PATCH_COMMAND ./autogen.sh CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-module-recovery --prefix=<INSTALL_DIR>
        BUILD_COMMAND make INSTALL_COMMAND make install #TEST_COMMAND make check
    )
    ExternalProject_Get_Property(secp256k1_submodule INSTALL_DIR)
    link_directories(libbtc INTERFACE ${INSTALL_DIR}/lib)
    include_directories(libbtc INTERFACE ${INSTALL_DIR}/include)

    find_library(gmp gmp REQUIRED)
    pkg_check_modules(libevent REQUIRED IMPORTED_TARGET libevent)
    
    add_dependencies(libbtc secp256k1_submodule libbtc_submodule)
    target_link_libraries(libbtc INTERFACE btc secp256k1 gmp PkgConfig::libevent)

add_library(onchain src/chain.cpp src/chain_libbtc.cpp)
target_link_libraries(onchain PRIVATE Threads::Threads libbtc)# PkgConfig::libsecp256k1 PkgConfig::libbtc)
target_include_directories(onchain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})# ${INSTALL_DIR}/include)

add_executable(onchain-bin src/tools/onchain.cpp)
target_link_libraries(onchain-bin onchain)
set_target_properties(onchain-bin PROPERTIES OUTPUT_NAME onchain)
