# Look for the header file.
find_path(__LightWebsockets_INCLUDE_DIR libwebsockets.h)
# Look for the library.
find_library(__LightWebsockets_LIBRARIES NAMES websockets)

# Handle the QUIETLY and REQUIRED arguments and set LightWebsockets_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LightWebsockets DEFAULT_MSG __LightWebsockets_LIBRARIES __LightWebsockets_INCLUDE_DIR)
# Copy the results to the output variables.

if(LightWebsockets_FOUND)
  set(LightWebsockets_INCLUDE_DIRS ${__LightWebsockets_INCLUDE_DIR})
	set(LightWebsockets_LIBRARIES ${__LightWebsockets_LIBRARIES})
else()
  set(LightWebsockets_INCLUDE_DIRS)
	set(LightWebsockets_LIBRARIES)
endif()
