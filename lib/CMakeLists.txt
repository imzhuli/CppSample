project(lib_${libname})

##FindPackages here

# enable specific macors by system type:
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(PLATFORM_LIBRARIES)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(PLATFORM_LIBRARIES)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	find_library(IOKIT_LIBRARY IOKit REQUIRED)
	set(PLATFORM_LIBRARIES ${IOKIT_LIBRARY} ${COCOA_LIBRARY})
endif()

# local dependencies
# paths
include_directories(
	./include/
	${OPENSSL_INCLUDE_DIR}
	)
link_directories(
	)

# libs:
set(shared_lib_dependency_private
	)
set(shared_lib_dependency_public
	${OPENSSL_LIBRARIES}
	${PLATFORM_LIBRARIES}
	${cpp})


# source files:
aux_source_directory(./src/private_lua                             ZEC_SRC)

aux_source_directory(./src/private_zec                             ZEC_SRC)
aux_source_directory(./src/private_zec/Algorithm                   ZEC_SRC)
aux_source_directory(./src/private_zec/Util                        ZEC_SRC)
aux_source_directory(./src/private_zec/Container                   ZEC_SRC)
aux_source_directory(./src/private_zec_ext/Lua                     ZEC_SRC)

aux_source_directory(./src/private_zec_ext/IO                      ZEC_SRC)
aux_source_directory(./src/private_zec_ext/IO/Darwin               ZEC_SRC)
aux_source_directory(./src/private_zec_ext/IO/Linux                ZEC_SRC)
aux_source_directory(./src/private_zec_ext/IO/Windows              ZEC_SRC)
aux_source_directory(./src/private_zec_ext/Utility                 ZEC_SRC)

aux_source_directory(./src/private_zec_game/Base                   ZEC_SRC)
aux_source_directory(./src/private_zec_game/Window                 ZEC_SRC)

# 3rd party

# C version utils:
aux_source_directory(./src/private_zec/C                           ZEC_SRC)


if (ZEC_OPTION_STATIC)
#static lib:
	add_library(${libname} ${ZEC_SRC})
	set_target_properties(${libname} PROPERTIES VERSION 1.0)
	target_link_libraries(${libname} PRIVATE ${shared_lib_dependency_private})
	target_link_libraries(${libname} PUBLIC ${shared_lib_dependency_public})
	target_compile_definitions(${libname} PRIVATE ZEC_OPTION_STATIC)

else()
#dynamic lib:
	add_library(${libname} SHARED  ${ZEC_SRC})
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
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/./include/zec_ext
	DESTINATION include
	FILES_MATCHING REGEX ".+\.(h|hpp)"
	)
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/./include/glm
	DESTINATION include
	FILES_MATCHING REGEX ".+\.(h|hpp|inc)"
	)
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/./include/rapidjson
	DESTINATION include
	FILES_MATCHING REGEX ".+\.(h|hpp)"
	)
install(TARGETS
	${libname}
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)
