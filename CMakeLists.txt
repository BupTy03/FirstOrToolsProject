cmake_minimum_required(VERSION 3.17)
project(FirstOrToolsProject)

set(CMAKE_CXX_STANDARD 17)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(
        REQUIRES fmt/6.0.0@bincrafters/stable
        BUILD missing)
conan_define_targets()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

add_executable(FirstOrToolsProject
        main.cpp ScheduleData.h
        ScheduleView.h ScheduleView.cpp
        ScheduleTask.h
        ScheduleCommon.h)

set(OR_TOOLS_ROOT D:/Libs/or-tools_VisualStudio2019-64bit_v8.1.8487)
target_include_directories(FirstOrToolsProject PRIVATE ${OR_TOOLS_ROOT}/include)
target_link_directories(FirstOrToolsProject PRIVATE ${OR_TOOLS_ROOT}/lib)
target_link_libraries(FirstOrToolsProject PRIVATE ortools CONAN_PKG::fmt)
