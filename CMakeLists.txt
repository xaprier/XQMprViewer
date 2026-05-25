cmake_minimum_required(VERSION 3.20)
project(
    XQMprViewer
    VERSION 1.0.0
    LANGUAGES C CXX
    DESCRIPTION "An open-source MPR DICOM viewer built with modern C++ architecture, Qt6, and VTK rendering."
    HOMEPAGE_URL "https://github.com/xaprier/XQMprViewer"
)

set(PROJECT_ORGANIZATION "xaprier")
set(PROJECT_DOCUMENTATION "https://xaprier.dev/XQMprViewer")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Embed rpath so binaries find Qt 6.10.2, VTK, and the project's own shared lib
# without requiring LD_LIBRARY_PATH
if(DEFINED CMAKE_PREFIX_PATH)
    foreach(_prefix ${CMAKE_PREFIX_PATH})
        if(EXISTS "${_prefix}/lib")
            list(APPEND CMAKE_INSTALL_RPATH "${_prefix}/lib")
        endif()
    endforeach()
endif()
list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_BINARY_DIR}/lib" "/usr/local/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# MPI must be found before VTK so VTK::mpi can resolve MPI::MPI_C
find_package(MPI QUIET)

find_package(VTK REQUIRED COMPONENTS
    RenderingOpenGL2
    RenderingAnnotation
    RenderingCore
    GUISupportQt
    InteractionWidgets
    InteractionStyle
    InteractionImage
    IOImage
    FiltersCore
    FiltersSources
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

find_package(DICOM QUIET)
if(NOT DICOM_FOUND)
    message(FATAL_ERROR "VTK-DICOM not found.")
endif()
if(DICOM_USE_FILE)
    include(${DICOM_USE_FILE})
endif()
set(VTK_DICOM_LIBRARIES vtkDICOM-${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR})
if(DEFINED VTK_DICOM_ROOT AND EXISTS "${VTK_DICOM_ROOT}/include/vtk-${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR}")
    list(APPEND VTK_DICOM_INCLUDE_DIRS "${VTK_DICOM_ROOT}/include/vtk-${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR}")
endif()
link_directories(/usr/local/lib)
if(DEFINED VTK_DICOM_ROOT AND EXISTS "${VTK_DICOM_ROOT}/lib")
    link_directories("${VTK_DICOM_ROOT}/lib")
endif()

add_subdirectory(lib)
add_subdirectory(src)

# compile_commands.json symlink to project root
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(link_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
        COMMENT "Linking compile_commands.json to project root"
    )
endif()
