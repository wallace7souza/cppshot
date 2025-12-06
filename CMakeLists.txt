cmake_minimum_required(VERSION 4.0)
project(cppshot)

set(CMAKE_CXX_STANDARD 20)

# Find wxWidgets and configure include/link flags
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

add_executable(cppshot main.cpp
        ScreenshotEditorFrame.h
        ScreenshotEditorFrame.cpp
)

target_link_libraries(cppshot PRIVATE ${wxWidgets_LIBRARIES})
