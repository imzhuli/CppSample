# Look for the header file.
find_path(MBED_INCLUDE_DIR mbedtls/ssl.h)
# Look for the library.
find_library(MBED_LIBRARY NAMES mbedcrypto mbedtls mbedx509)

# Handle the QUIETLY and REQUIRED arguments and set MBED_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MBED DEFAULT_MSG MBED_LIBRARY MBED_INCLUDE_DIR)
# Copy the results to the output variables.

if(MBED_FOUND)
	set(MBED_LIBRARIES ${MBED_LIBRARY})
	set(MBED_INCLUDE_DIRS ${MBED_INCLUDE_DIR})
else()
	set(MBED_LIBRARIES)
	set(MBED_INCLUDE_DIRS)
endif()
