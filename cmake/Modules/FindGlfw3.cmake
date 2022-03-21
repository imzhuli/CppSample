# Look for the header file.
find_path(__GLFW3_INCLUDE_DIR GLFW/glfw3.h)
# Look for the library.
find_library(__GLFW3_LIBRARIES NAMES glfw)

# Handle the QUIETLY and REQUIRED arguments and set GLFW3_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLFW3 DEFAULT_MSG __GLFW3_LIBRARIES __GLFW3_INCLUDE_DIR)
# Copy the results to the output variables.

if(GLFW3_FOUND)
  set(GLFW3_INCLUDE_DIRS ${__GLFW3_INCLUDE_DIR})
	set(GLFW3_LIBRARIES ${__GLFW3_LIBRARIES})
else()
  set(GLFW3_INCLUDE_DIRS)
	set(GLFW3_LIBRARIES)
endif()
