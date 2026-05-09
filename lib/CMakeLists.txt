cmake_minimum_required(VERSION 3.20)

add_library(XQVtkViewport SHARED
    include/XQVtkViewport/ViewportManager.hpp
    include/XQVtkViewport/ViewportLayout.hpp
    src/RenderStats.cpp
    src/RenderStatsOverlay.cpp
    src/ViewportManager.cpp
    src/ViewportLayout.cpp
)

include(GenerateExportHeader)
generate_export_header(XQVtkViewport
    BASE_NAME QVV
    EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/XQVtkViewport/XQVtkViewport_export.hpp
)

target_include_directories(XQVtkViewport
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

find_package(VTK REQUIRED COMPONENTS
    RenderingOpenGL2
    RenderingAnnotation
    RenderingCore
    GUISupportQt
    InteractionWidgets
    InteractionStyle
)
find_package(Qt6 REQUIRED COMPONENTS Core Gui)

target_link_libraries(XQVtkViewport
    PUBLIC
        ${VTK_LIBRARIES}
        Qt6::Core
        Qt6::Gui
)

target_compile_features(XQVtkViewport PUBLIC cxx_std_17)

vtk_module_autoinit(
    TARGETS XQVtkViewport
    MODULES ${VTK_LIBRARIES}
)

install(TARGETS XQVtkViewport EXPORT XQVtkViewportTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)
