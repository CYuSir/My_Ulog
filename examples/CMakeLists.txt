include(toolchain/aarch64)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -std=c++11 -std=c++17 -O3")

# Generate the escape_structs.hpp using a custom command
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/escape_structs.hpp
    COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/generate_structs.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMENT "Generating escape_structs.hpp"
    VERBATIM
)
message(DIR STATUS ${CMAKE_CURRENT_LIST_DIR})
# Create a custom target that depends on the generated file
add_custom_target(generate_structs DEPENDS ALL ${CMAKE_CURRENT_LIST_DIR}/escape_structs.hpp)

ZZ_MODULE(
	NAME logger_test
	TYPE APP
	VERSION
		${ZZLOG_VERSION}
	SOURCES
		logger_test.cpp
	INCS
		${LIB_ROOT_DIR}
	LINK_LIBS
		zz_data_log
		pthread
	COMPONENT
		core
	PKG logger_test
)

ZZ_MODULE(
	NAME ulog_data
	TYPE APP
	VERSION
		${ZZLOG_VERSION}
	SOURCES
		ulog_data.cpp
	INCS
		${LIB_ROOT_DIR}
	LINK_LIBS
		zz_data_log
	COMPONENT
		core
	PKG ulog_data
)

ZZ_MODULE(
	NAME ulog_info
	TYPE APP
	VERSION
		${ZZLOG_VERSION}
	SOURCES
		ulog_info.cpp
	INCS
		${LIB_ROOT_DIR}
	LINK_LIBS
		zz_data_log
	COMPONENT
		core
	PKG ulog_info
)

ZZ_MODULE(
	NAME ulog_writer
	TYPE APP
	VERSION
		${ZZLOG_VERSION}
	SOURCES
		ulog_writer.cpp
	INCS
		${LIB_ROOT_DIR}
	LINK_LIBS
		zz_data_log
	COMPONENT
		core
	PKG ulog_writer
)