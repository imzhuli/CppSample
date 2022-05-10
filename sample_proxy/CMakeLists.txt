project(sample_${libname})

# dependencies:

# local dependencies
include_directories(
	../lib/src/
	../lib/include/
	)
link_directories(
	)

set(shared_lib_dependency_private
	)

aux_source_directory(./base/    COMMON_SOURCE_CODE)
aux_source_directory(./common/  COMMON_SOURCE_CODE)

function(make_source sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(x_${sub_dir_name}  ${LOCAL_SOURCE_CODE} ${COMMON_SOURCE_CODE})
	target_link_libraries(x_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(x_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (ZEC_OPTION_STATIC)
	target_compile_definitions(x_${sub_dir_name} PRIVATE ZEC_OPTION_STATIC)
endif()
endfunction()

#####
make_source(dispatcher)
