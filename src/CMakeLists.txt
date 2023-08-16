
include(toolchain/aarch64)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -std=c++11 -std=c++17 -O3")

ZZ_MODULE(
	NAME zz_data_log
	TYPE SHARED
	VERSION
		${ZZLOG_VERSION}
	SOURCES
		${LIB_ROOT_DIR}/src/
	INCS
		${LIB_ROOT_DIR}/include/ulog_cpp
	INTERFACES
		${LIB_ROOT_DIR}/include
	LINK_LIBS
	COMPONENT
		core
	PKG zz_data_log
)