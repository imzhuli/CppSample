# Look for the header file.
find_path(__MYSQL_INCLUDE_DIR mysql/mysql.h)
# Look for the library.
find_library(__MYSQL_LIBRARIES NAMES libmysql mysqlclient HINTS /usr/lib64/mysql/)

# Handle the QUIETLY and REQUIRED arguments and set MYSQL_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MYSQL DEFAULT_MSG __MYSQL_LIBRARIES __MYSQL_INCLUDE_DIR)
# Copy the results to the output variables.

if(MYSQL_FOUND)
  set(MYSQL_INCLUDE_DIRS ${__MYSQL_INCLUDE_DIR})
	set(MYSQL_LIBRARIES ${__MYSQL_LIBRARIES})
else()
  set(MYSQL_INCLUDE_DIRS)
	set(MYSQL_LIBRARIES)
endif()
