cmake_minimum_required(VERSION 3.20)
project(XQVtkViewport VERSION 1.0.0 LANGUAGES C CXX)

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
set(VTK_DICOM_LIBRARIES vtkDICOM-9.6)
link_directories(/usr/local/lib)

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
