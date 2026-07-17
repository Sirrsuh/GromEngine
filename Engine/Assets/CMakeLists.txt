cmake_minimum_required(VERSION 3.20)
project(GromAssets LANGUAGES CXX)

if (GROM_RHI_D3D11 OR GROM_RHI_VULKAN)
    add_library(GromAssets STATIC Source/Assets.cpp)
else()
    add_library(GromAssets STATIC Source/Stub.cpp)
endif()

target_include_directories(GromAssets PUBLIC Include)
target_link_libraries(GromAssets PUBLIC GromCore GromJobs)
target_compile_features(GromAssets PUBLIC cxx_std_20)
