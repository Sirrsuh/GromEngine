cmake_minimum_required(VERSION 3.20)
project(GromScene LANGUAGES CXX)
add_library(GromScene STATIC
    Source/SceneNode.cpp
    Source/Scene.cpp
    Source/Mesh.cpp
    Source/Model.cpp
    Source/Camera.cpp
    Source/Light.cpp
)
target_include_directories(GromScene PUBLIC Include)
target_link_libraries(GromScene PUBLIC GromCore GromRHI GromMaterials GromJobs)
target_compile_features(GromScene PUBLIC cxx_std_20)
