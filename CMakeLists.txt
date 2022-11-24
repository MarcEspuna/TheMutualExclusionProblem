cmake_minimum_required(VERSION 3.7)
project(Processes VERSION 0.1.0)

include(CTest)
enable_testing()

add_compile_options(/MDd /MTd)

add_subdirectory(dependencies/spdlog)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

set(LIB_SRCS Utilities/CentMutex.cpp Utilities/Client.cpp Utilities/Server.cpp Utilities/Socket.cpp Utilities/ProcessLauncher.cpp Utilities/MsgHandler.cpp Utilities/Log.cpp Utilities/DirectClock.cpp Utilities/LamportMutex.cpp Utilities/RAMutex.cpp)
set(LIBS Ws2_32.lib spdlog::spdlog)

add_library(utilities STATIC ${LIB_SRCS})
target_link_libraries(utilities ${LIBS})
target_include_directories(utilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} dependencies/spdlog/include/spdlog)
target_compile_options(utilities PUBLIC "/std:c++17")

add_executable(App main.cpp App.cpp ${LIB_SRCS})
target_link_libraries(App utilities)
target_compile_options(App PUBLIC "/std:c++17")
target_include_directories(App PUBLIC Utilities dependencies/spdlog/include/spdlog)

add_executable(HW HeavyWeights/main.cpp HeavyWeights/HWApp.cpp ${LIB_SRCS})
target_link_libraries(HW utilities)
target_compile_options(HW PUBLIC "/std:c++17")
target_include_directories(HW PUBLIC Utilities dependencies/spdlog/include/spdlog)

add_executable(LW LightWeights/main.cpp LightWeights/LWApp.cpp ${LIB_SRCS})
target_link_libraries(LW utilities)
target_compile_options(LW PUBLIC "/std:c++17")
target_include_directories(LW PUBLIC Utilities dependencies/spdlog/include/spdlog)

install( TARGETS utilities App HW LW)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
