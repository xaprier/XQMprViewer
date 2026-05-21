cmake_minimum_required(VERSION 3.20)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/app/include/app/Version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/app/include/app/Version.hpp"
    @ONLY
)

file (GLOB_RECURSE CPP_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/adapters/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/controllers/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/overlays/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/render/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/src/*.cpp"
)

file (GLOB_RECURSE HPP_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/adapters/include/adapters/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/include/app/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/controllers/include/controllers/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/overlays/include/overlays/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/render/include/render/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/include/ui/*.hpp"
)

add_executable(XQVtkViewport_Widgets
    ${CPP_FILES}
    ${HPP_FILES}
)

target_include_directories(XQVtkViewport_Widgets
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/adapters/include
        ${CMAKE_CURRENT_SOURCE_DIR}/app/include
        ${CMAKE_CURRENT_BINARY_DIR}/app/include
        ${CMAKE_CURRENT_SOURCE_DIR}/controllers/include
        ${CMAKE_CURRENT_SOURCE_DIR}/overlays/include
        ${CMAKE_CURRENT_SOURCE_DIR}/render/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/include
)

target_link_libraries(XQVtkViewport_Widgets
    PRIVATE
        XQVtkViewport
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        VTK::GUISupportQt
        VTK::RenderingOpenGL2
        VTK::InteractionStyle
        VTK::InteractionWidgets
        VTK::RenderingAnnotation
        ${VTK_DICOM_LIBRARIES}
        ${VTK_LIBRARIES}
)

target_compile_features(XQVtkViewport_Widgets PRIVATE cxx_std_17)

vtk_module_autoinit(
    TARGETS XQVtkViewport_Widgets
    MODULES
        VTK::GUISupportQt
        VTK::RenderingOpenGL2
        VTK::InteractionStyle
        VTK::InteractionWidgets
        VTK::RenderingAnnotation
        ${VTK_LIBRARIES}
)
