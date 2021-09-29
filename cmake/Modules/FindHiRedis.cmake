# Look for the header file.
find_path(__HiRedis_INCLUDE_DIR hiredis/hiredis.h)
# Look for the library.
find_library(__HiRedis_LIBRARIES NAMES hiredis)

# Handle the QUIETLY and REQUIRED arguments and set HiRedis_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HiRedis DEFAULT_MSG __HiRedis_LIBRARIES __HiRedis_INCLUDE_DIR)
# Copy the results to the output variables.

if(HiRedis_FOUND)
  set(HiRedis_INCLUDE_DIRS ${__HiRedis_INCLUDE_DIR})
	set(HiRedis_LIBRARIES ${__HiRedis_LIBRARIES})
else()
  set(HiRedis_INCLUDE_DIRS)
	set(HiRedis_LIBRARIES)
endif()
