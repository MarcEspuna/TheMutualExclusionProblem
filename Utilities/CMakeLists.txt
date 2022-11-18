cmake_minimum_required(VERSION 3.7)
project(Utilities VERSION 0.1.0)

include(CTest)
enable_testing()

add_compile_options(/MDd /MTd)

add_library(Utilities Process.cpp CentMutex.cpp Client.cpp Server.cpp Socket.cpp MsgHandler.cpp)

target_compile_options(Utilities PUBLIC "/std:c++17")
target_link_libraries(Utilities Ws2_32.lib)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
