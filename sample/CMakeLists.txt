project(sample_${libname})

find_package(FastCGI)
find_package(Lua REQUIRED)

# dependencies:
set(shared_lib_dependency_private)

# local dependencies
include_directories(
	../lib/src/
	../lib/include/
	${FCGI_INCLUDE_DIRS}
	${LUA_INCLUDE_DIR}
	)
link_directories(
	)

set(shared_lib_dependency_private
	${shared_lib_dependency_private}
	${FCGI_LIBRARIES}
	${LUA_LIBRARIES}
)

function(add_sub_sample sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(sample_${sub_dir_name}  ${LOCAL_SOURCE_CODE})
	target_link_libraries(sample_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(sample_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (ZEC_OPTION_STATIC)
	target_compile_definitions(sample_${sub_dir_name} PRIVATE ZEC_OPTION_STATIC)
endif()
endfunction()

#test lib:

add_sub_sample(app)
add_sub_sample(get_mac)
add_sub_sample(http_echo)
add_sub_sample(lua)


