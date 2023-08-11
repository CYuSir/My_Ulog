set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Generate the escape_structs.hpp using a custom command
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/escape_structs.hpp
    COMMAND python3 ${CMAKE_SOURCE_DIR}/examples/generate_structs.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating escape_structs.hpp"
    VERBATIM
)

# Create a custom target that depends on the generated file
add_custom_target(generate_structs DEPENDS ${CMAKE_BINARY_DIR}/escape_structs.hpp)

add_executable(ulog_info ulog_info.cpp)
target_link_libraries(ulog_info PUBLIC
		zz_data_logger
		)

add_executable(ulog_data ulog_data.cpp)
target_link_libraries(ulog_data PUBLIC
		zz_data_logger
		)

add_executable(ulog_writer ulog_writer.cpp)
target_link_libraries(ulog_writer PUBLIC
		zz_data_logger
		)

add_executable(logger_test logger_test.cpp)
target_link_libraries(logger_test PUBLIC
		zz_data_logger
		pthread
		)

add_dependencies(logger_test generate_structs)
