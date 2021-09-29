project(lib_${libname})

##FindPackages here

# enable specific macors by system type:
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	#Platform
	set(PLATFORM_LIBRARIES)
	#end of Platform
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	#Platform
	set(PLATFORM_LIBRARIES)
	#end of Platform
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	#Platform
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	find_library(IOKIT_LIBRARY IOKit REQUIRED)
	set(PLATFORM_LIBRARIES ${IOKIT_LIBRARY} ${COCOA_LIBRARY})
	#end of Platform
endif()

# local dependencies
# paths
include_directories(
	./include/
	)
link_directories(
	)

# libs:
set(shared_lib_dependency_private)
set(shared_lib_dependency_public
	${PLATFORM_LIBRARIES}
	${cpp})


# source files:
aux_source_directory(./src/private_zec                             ZEC_SRC)
aux_source_directory(./src/private_zec/Util                        ZEC_SRC)
aux_source_directory(./src/private_zec/Container                   ZEC_SRC)

if (ZEC_OPTION_STATIC)
#static lib:
	add_library(${libname} ${ZEC_SRC} ${ZEC_NET_SRC} ${ZEC_MYSQL_SRC} ${ZEC_GAME_SRC})
	set_target_properties(${libname} PROPERTIES VERSION 1.0)
	target_link_libraries(${libname} PRIVATE ${shared_lib_dependency_private})
	target_link_libraries(${libname} PUBLIC ${shared_lib_dependency_public})
	target_compile_definitions(${libname} PRIVATE ZEC_OPTION_STATIC)

else()
#dynamic lib:
	add_library(${libname} SHARED  ${ZEC_SRC} ${ZEC_NET_SRC} ${ZEC_MYSQL_SRC} ${ZEC_GAME_SRC})
	set_target_properties(${libname} PROPERTIES VERSION 1.0)
	target_link_libraries(${libname} PRIVATE ${shared_lib_dependency_private})
	target_link_libraries(${libname} PUBLIC ${shared_lib_dependency_public})
	target_compile_definitions(${libname} PRIVATE ZEC_OPTION_EXPORT_API)
endif()

#install
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/./include/zec
	DESTINATION include
	FILES_MATCHING REGEX ".+\.(h|hpp)"
	)
install(TARGETS
	zec
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)
