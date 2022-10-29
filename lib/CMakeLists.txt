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
aux_source_directory(./src/private_lua                             X_SRC)

aux_source_directory(./src/private_xel                             X_SRC)
aux_source_directory(./src/private_xel/Algorithm                   X_SRC)
aux_source_directory(./src/private_xel/Util                        X_SRC)
aux_source_directory(./src/private_xel/Container                   X_SRC)
aux_source_directory(./src/private_xel_ext/Lua                     X_SRC)

aux_source_directory(./src/private_xel_ext/IO                      X_SRC)
# aux_source_directory(./src/private_xel_ext/IO/Darwin               X_SRC)
# aux_source_directory(./src/private_xel_ext/IO/Linux                X_SRC)
# aux_source_directory(./src/private_xel_ext/IO/Windows              X_SRC)
aux_source_directory(./src/private_xel_ext/Utility                 X_SRC)

aux_source_directory(./src/private_xel_game/Base                   X_SRC)
aux_source_directory(./src/private_xel_game/Window                 X_SRC)

# 3rd party

if (X_OPTION_STATIC)
#static lib:
	add_library(${libname} ${X_SRC})
	set_target_properties(${libname} PROPERTIES VERSION 1.0)
	target_link_libraries(${libname} PRIVATE ${shared_lib_dependency_private})
	target_link_libraries(${libname} PUBLIC ${shared_lib_dependency_public})
	target_compile_definitions(${libname} PRIVATE X_OPTION_STATIC)

else()
#dynamic lib:
	add_library(${libname} SHARED  ${X_SRC})
	set_target_properties(${libname} PROPERTIES VERSION 1.0)
	target_link_libraries(${libname} PRIVATE ${shared_lib_dependency_private})
	target_link_libraries(${libname} PUBLIC ${shared_lib_dependency_public})
	target_compile_definitions(${libname} PRIVATE X_OPTION_EXPORT_API)
endif()

#install
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/./include/xel
	DESTINATION include
	FILES_MATCHING REGEX ".+\.(h|hpp)"
	)
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/./include/xel_ext
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
