cmake_minimum_required(VERSION 3.20)
project(GromMaterials LANGUAGES CXX)
add_library(GromMaterials STATIC
    Source/Material.cpp
    Source/MaterialInstance.cpp
    Source/MaterialParameterBlock.cpp
    Source/ShaderPermutation.cpp
    Source/ShaderLibrary.cpp
)
target_include_directories(GromMaterials PUBLIC Include)
target_link_libraries(GromMaterials PUBLIC GromCore GromRHI GromJobs)
target_compile_features(GromMaterials PUBLIC cxx_std_20)
