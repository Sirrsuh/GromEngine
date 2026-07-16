cmake_minimum_required(VERSION 3.20)
project(GromUI LANGUAGES CXX)
add_library(GromUI STATIC
    Source/UIElement.cpp
    Source/UIWidget.cpp
    Source/UICanvas.cpp
    Source/UILayout.cpp
    Source/UIButton.cpp
    Source/UIText.cpp
    Source/UIPanel.cpp
)
target_include_directories(GromUI PUBLIC Include)
target_link_libraries(GromUI PUBLIC GromCore GromRHI)
target_compile_features(GromUI PUBLIC cxx_std_20)
