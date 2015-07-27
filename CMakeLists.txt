cmake_minimum_required(VERSION 2.8)
 
PROJECT(GamepadInteraction)

#SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g -std=c++11")

option(WRAP_PYTHON "Build Python wrappers" ON)
 
find_package(VTK REQUIRED 
    vtkInteractionStyle 
    vtkRenderingCore vtkRenderingOpenGL2 
    vtkWrappingPythonCore)
    
include(${VTK_USE_FILE})

if (WRAP_PYTHON)
    find_package(PythonLibs 2.7 EXACT REQUIRED)
    include(vtkWrapPython)
endif()

SET(LIBS ${VTK_LIBRARIES})

#IF(UNIX)
#    SET(LIBS ${LIBS} pthread udev rt X11 Xinerama Xxf86vm Xrandr)
#ENDIF(UNIX)

add_subdirectory(src)
