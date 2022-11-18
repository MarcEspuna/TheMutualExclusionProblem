cmake_minimum_required(VERSION 3.7)
project(Processes VERSION 0.1.0)

include(CTest)
enable_testing()

add_compile_options(/MDd /MTd)

set(LIB_SRCS Utilities/CentMutex.cpp Utilities/Client.cpp Utilities/Server.cpp Utilities/Socket.cpp Utilities/Process.cpp Utilities/MsgHandler.cpp)

add_library(utilities STATIC ${LIB_SRCS})
target_link_libraries(utilities Ws2_32.lib)
target_include_directories(utilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(App main.cpp App.cpp ${LIB_SRCS})
target_link_libraries(App utilities)
target_compile_options(App PUBLIC "/std:c++17")
target_include_directories(App PUBLIC Utilities)

add_executable(HW HeavyWeights/main.cpp ${LIB_SRCS})
target_link_libraries(HW utilities)
target_compile_options(HW PUBLIC "/std:c++17")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} Utilities)

install( TARGETS utilities App HW )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
