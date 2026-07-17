cmake_minimum_required(VERSION 3.20)
project(GromAssets LANGUAGES CXX)

if (GROM_RHI_D3D11 OR GROM_RHI_VULKAN)
    add_library(GromAssets STATIC Source/Assets.cpp Source/Package.cpp)
    target_link_libraries(GromAssets PUBLIC GromCore GromJobs GromRHI)
else()
    add_library(GromAssets STATIC Source/Stub.cpp)
    target_link_libraries(GromAssets PUBLIC GromCore GromJobs)
endif()

target_include_directories(GromAssets PUBLIC Include)
target_compile_features(GromAssets PUBLIC cxx_std_20)
