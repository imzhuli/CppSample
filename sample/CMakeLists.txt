project(sample_${libname})

find_package(Lua REQUIRED)

# local dependencies
include_directories(
	../lib/src/
	../lib/include/
	${LUA_INCLUDE_DIR}
	)
link_directories(
	)

set(shared_lib_dependency_private
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

add_sub_sample(glm)
if (ZEC_OPTION_WITH_LUA)
	add_sub_sample(lua)
endif()
add_sub_sample(recent_logs)
add_sub_sample(tcp_echo)
add_sub_sample(udp_echo)

install(TARGETS
	sample_tcp_echo
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)
