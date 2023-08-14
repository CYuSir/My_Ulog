
add_library(${PROJECT_NAME}
	data_container.cpp
	messages.cpp
	reader.cpp
	writer.cpp
	simple_writer.cpp
	zz_data_log.cpp
)
# add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
add_library(zz_data_logger ALIAS ${PROJECT_NAME})

