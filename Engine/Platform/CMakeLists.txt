cmake_minimum_required(VERSION 3.20)
project(GromPlatform LANGUAGES CXX OBJCXX)

add_library(GromPlatform STATIC)

target_include_directories(GromPlatform PUBLIC Include)

if(GROM_PLATFORM_WIN32)
    target_sources(GromPlatform PRIVATE
        Source/Win32/Win32_Window.cpp
        Source/Win32/Win32_Platform.cpp
    )
elseif(GROM_PLATFORM_LINUX)
    target_sources(GromPlatform PRIVATE
        Source/Linux/Linux_Window.cpp
        Source/Linux/Linux_Platform.cpp
    )
    find_package(X11 REQUIRED)
    target_link_libraries(GromPlatform PUBLIC ${X11_LIBRARIES})
elseif(GROM_PLATFORM_MACOS)
    target_sources(GromPlatform PRIVATE
        Source/macOS/macOS_Window.mm
        Source/macOS/macOS_Platform.mm
    )
    find_library(COCOA Cocoa)
    find_library(METAL Metal)
    find_library(QUARTZCORE QuartzCore)
    target_link_libraries(GromPlatform PUBLIC ${COCOA} ${METAL} ${QUARTZCORE})
endif()

target_link_libraries(GromPlatform PUBLIC GromCore)
