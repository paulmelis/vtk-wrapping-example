include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(Gamepad_SRCS 
    vtkInteractorStyleGame
    GamepadHandler)
    
# Don't generate wrapper for this file, as it doesn't derive from vtkObject,
# so VTK doesn't know how to wrap it (nor do we need it wrapper anyway).

set_source_files_properties(
   GamepadHandler
   WRAP_EXCLUDE)    

# A library containing all stuff

add_library(vtkGamepadLib ${Gamepad_SRCS})

# Python wrapping

if (WRAP_PYTHON)

    include_directories("${PYTHON_INCLUDE_PATH}")

    # Generate wrapper code
    vtk_wrap_python3(vtkGamepadPython GamepadPython_SRCS "${Gamepad_SRCS}")
    
    add_library(vtkGamepadPythonD ${GamepadPython_SRCS} ${Gamepad_SRCS})    
    
    target_link_libraries(vtkGamepadPythonD         
        ${VTK_LIBRARIES}
        vtkWrappingPythonCore
        ${VTK_PYTHON_LIBRARIES})
    
    add_library(vtkGamepadPython MODULE vtkGamepadPythonInit.cxx)
    
    target_link_libraries(vtkGamepadPython vtkGamepadPythonD)

    set_target_properties(vtkGamepadPython PROPERTIES PREFIX "")
    if(WIN32 AND NOT CYGWIN)
        SET_TARGET_PROPERTIES(vtkGamepadPython PROPERTIES SUFFIX ".pyd")
    endif(WIN32 AND NOT CYGWIN)

endif()