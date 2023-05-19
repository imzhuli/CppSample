project(test_${libname})

# dependencies:

# local dependencies
include_directories(
	../lib/include/
	../lib/src/
	)
link_directories(
	)
set(shared_lib_dependency_private
	${LightWebsockets_LIBRARIES}
)

function(build_sub sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(test_${sub_dir_name}  ${LOCAL_SOURCE_CODE})
	target_link_libraries(test_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(test_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (X_OPTION_STATIC)
	target_compile_definitions(test_${sub_dir_name} PRIVATE X_OPTION_STATIC)
endif()
endfunction()

function(test_sub sub_dir_name)
	add_test(
		NAME	test_${sub_dir_name}
		COMMAND test_${sub_dir_name} ${args}
		WORKING_DIRECTORY  ${CMAKE_SOURCE_DIR}
		)
endfunction()

#test lib:
build_sub(_)
build_sub(dns)


test_sub(_)


