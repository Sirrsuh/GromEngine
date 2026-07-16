cmake_minimum_required(VERSION 3.20)
project(GromRenderer LANGUAGES CXX)
add_library(GromRenderer STATIC
    Source/Renderer.cpp
)
target_include_directories(GromRenderer PUBLIC Include)
target_link_libraries(GromRenderer PUBLIC GromCore GromRHI GromScene GromMaterials GromJobs)
target_compile_features(GromRenderer PUBLIC cxx_std_20)
