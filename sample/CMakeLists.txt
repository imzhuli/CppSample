project(sample_${libname})

# local dependencies
include_directories(
	../lib/src/
	../lib/include/
	)
link_directories(
	)

set(shared_lib_dependency_private
)

function(add_sub_sample sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(sample_${sub_dir_name}  ${LOCAL_SOURCE_CODE})
	target_link_libraries(sample_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(sample_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (X_OPTION_STATIC)
	target_compile_definitions(sample_${sub_dir_name} PRIVATE X_OPTION_STATIC)
endif()
endfunction()

add_sub_sample(tcp_client)
add_sub_sample(tcp_server)

