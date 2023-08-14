
add_library(${PROJECT_NAME} SHARED
	data_container.cpp
	messages.cpp
	reader.cpp
	writer.cpp
	simple_writer.cpp
	zz_data_log.cpp
)

target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>)

# add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
add_library(zz_data_logger ALIAS ${PROJECT_NAME})

