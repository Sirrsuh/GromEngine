cmake_minimum_required(VERSION 3.20)
project(GromRHI LANGUAGES CXX)

add_library(GromRHI STATIC)

target_include_directories(GromRHI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Backend
)

if(GROM_RHI_D3D11)
    target_compile_definitions(GromRHI PUBLIC GROM_RHI_D3D11=1)
    target_sources(GromRHI PRIVATE
        Source/RHI_Device.cpp
        Source/RHI_Shader.cpp
        Source/RHI_Buffer.cpp
        Source/RHI_Texture.cpp
        Source/RHI_Pipeline.cpp
        Source/Backend/D3D11/D3D11_Device.cpp
        Source/Backend/D3D11/D3D11_Shader.cpp
        Source/Backend/D3D11/D3D11_Buffer.cpp
        Source/Backend/D3D11/D3D11_Texture.cpp
        Source/Backend/D3D11/D3D11_Pipeline.cpp
    )
endif()

if(GROM_RHI_D3D12)
    target_compile_definitions(GromRHI PUBLIC GROM_RHI_D3D12=1)
    target_sources(GromRHI PRIVATE
        Source/Backend/D3D12/D3D12_Device.cpp
        Source/Backend/D3D12/D3D12_Shader.cpp
        Source/Backend/D3D12/D3D12_Buffer.cpp
        Source/Backend/D3D12/D3D12_Texture.cpp
        Source/Backend/D3D12/D3D12_Pipeline.cpp
    )
    target_link_libraries(GromRHI PUBLIC d3d12.lib dxgi.lib d3dcompiler.lib)
endif()

if(GROM_RHI_VULKAN)
    target_compile_definitions(GromRHI PUBLIC GROM_RHI_VULKAN=1)
    target_sources(GromRHI PRIVATE
        Source/Backend/Vulkan/Vulkan_Device.cpp
        Source/Backend/Vulkan/Vulkan_Shader.cpp
        Source/Backend/Vulkan/Vulkan_Buffer.cpp
        Source/Backend/Vulkan/Vulkan_Texture.cpp
        Source/Backend/Vulkan/Vulkan_Pipeline.cpp
    )
    find_package(Vulkan REQUIRED)
    
    # VMA (Vulkan Memory Allocator) - header only, VMA_IMPLEMENTATION only in Vulkan_Device.cpp
    target_include_directories(GromRHI PRIVATE ${CMAKE_SOURCE_DIR}/ThirdParty/VMA/include)
    
    target_link_libraries(GromRHI PUBLIC Vulkan::Vulkan)
endif()

if(GROM_RHI_OPENGL)
    target_compile_definitions(GromRHI PUBLIC GROM_RHI_OPENGL=1)
endif()

target_link_libraries(GromRHI
    PUBLIC
        GromCore
        d3d11.lib
        d3dcompiler.lib
        dxgi.lib
)

target_compile_features(GromRHI PUBLIC cxx_std_20)
