cmake_minimum_required(VERSION 3.11)

project(libonchain)

find_package(Threads REQUIRED)
find_package(PkgConfig)
include(ExternalProject)

execute_process(COMMAND git submodule update --init --recursive WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

SET(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "Run cpplint.py tool within SQLiteC++")
add_subdirectory(deps/sqlitecpp)

add_library(libbtc INTERFACE)
    ExternalProject_add(
        libbtc_submodule SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/libbtc
        PATCH_COMMAND ./autogen.sh CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
        BUILD_COMMAND make INSTALL_COMMAND make install #TEST_COMMAND make check
    )
    ExternalProject_Get_Property(libbtc_submodule INSTALL_DIR)
    link_directories(libbtc INTERFACE ${INSTALL_DIR}/lib)
    include_directories(libbtc INTERFACE ${INSTALL_DIR}/include)

    pkg_check_modules(libevent REQUIRED IMPORTED_TARGET libevent)
    
    add_dependencies(libbtc libbtc_submodule libbtc_submodule)
    target_link_libraries(libbtc INTERFACE btc PkgConfig::libevent)

add_library(onchain src/chain.cpp src/chain_libbtc.cpp src/data.cpp src/data_sqlite.cpp)
target_link_libraries(onchain PRIVATE Threads::Threads libbtc SQLiteCpp)
target_include_directories(onchain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(onchain PUBLIC -Wall -pedantic -Werror -Wno-unused-variable)

add_executable(onchain-bin src/tools/onchain.cpp)
target_link_libraries(onchain-bin onchain)
set_target_properties(onchain-bin PROPERTIES OUTPUT_NAME onchain)

include(CTest)
if(BUILD_TESTING)
    add_executable(test-sqlite test/sqlite.cpp)
    target_link_libraries(test-sqlite onchain)
    add_test(NAME sqlite COMMAND test-sqlite)
endif()
