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

file (GLOB_RECURSE QRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/resources/*.qrc"
)

add_executable(XQMprViewer
    ${CPP_FILES}
    ${HPP_FILES}
    ${QRC_FILES}
)

if(WIN32)
    target_sources(XQMprViewer PRIVATE "${CMAKE_SOURCE_DIR}/assets/icon.rc")
endif()

target_include_directories(XQMprViewer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/adapters/include
        ${CMAKE_CURRENT_SOURCE_DIR}/app/include
        ${CMAKE_CURRENT_BINARY_DIR}/app/include
        ${CMAKE_CURRENT_SOURCE_DIR}/controllers/include
        ${CMAKE_CURRENT_SOURCE_DIR}/overlays/include
        ${CMAKE_CURRENT_SOURCE_DIR}/render/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/include
        ${VTK_DICOM_INCLUDE_DIRS}
)

target_link_libraries(XQMprViewer
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

target_compile_features(XQMprViewer PRIVATE cxx_std_17)

vtk_module_autoinit(
    TARGETS XQMprViewer
    MODULES
        VTK::GUISupportQt
        VTK::RenderingOpenGL2
        VTK::InteractionStyle
        VTK::InteractionWidgets
        VTK::RenderingAnnotation
        ${VTK_LIBRARIES}
)

install(TARGETS XQMprViewer
    RUNTIME DESTINATION bin
)

if(UNIX AND NOT APPLE)
    install(FILES "${CMAKE_SOURCE_DIR}/assets/XQMprViewer.desktop"
        DESTINATION share/applications
    )
    install(FILES "${CMAKE_SOURCE_DIR}/assets/XQMprViewer.png"
        DESTINATION share/icons/hicolor/1024x1024/apps
    )
endif()
