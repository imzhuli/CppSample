project(test_${libname})

# dependencies:
set(shared_lib_dependency_private ${LightWebsockets_LIBRARIES})

# local dependencies
include_directories(
	../lib/include/
	../lib/src/
	)
link_directories(
	)

function(add_sub_test sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(test_${sub_dir_name}  ${LOCAL_SOURCE_CODE})
	target_link_libraries(test_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(test_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (ZEC_OPTION_STATIC)
	target_compile_definitions(test_${sub_dir_name} PRIVATE ZEC_OPTION_STATIC)
endif()
	add_test(
		NAME	test_${sub_dir_name}
		COMMAND test_${sub_dir_name}
		WORKING_DIRECTORY  ${CMAKE_SOURCE_DIR}
		)
endfunction()

function(add_sub_buildonly sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(test_${sub_dir_name}  ${LOCAL_SOURCE_CODE})
	target_link_libraries(test_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(test_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (ZEC_OPTION_STATIC)
	target_compile_definitions(test_${sub_dir_name} PRIVATE ZEC_OPTION_STATIC)
endif()
endfunction()

#test lib:
add_sub_test(alignment)
add_sub_test(cmd)
add_sub_test(memlog)
add_sub_test(optional)
add_sub_test(rbtree)
add_sub_test(uuid)
#add_sub_test(thread)

add_sub_buildonly(_)
add_sub_buildonly(connection)

add_test(
	NAME cmd_short_long_empty
	COMMAND test_cmd "-abcv" "Value for V" "Arg1" "Arg2" "--EmptyValue"
	)
add_test(
	NAME cmd_short_long_subcmd
	COMMAND test_cmd "-a" "-bv" "Value for V" "-c" "Arg1" "--LongValue" "LongValue" "-e" "EmptyValue" "Arg2" "--" "SubCmd1" "SubCmd2"
	)