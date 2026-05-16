# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources
	${CMAKE_SOURCE_DIR}/src/*.*
	${CMAKE_SOURCE_DIR}/core/*.*
	${CMAKE_SOURCE_DIR}/drivers/*.*
	${CMAKE_SOURCE_DIR}/services/*.*
)

idf_component_register(
	SRCS ${app_sources}
	INCLUDE_DIRS
		${CMAKE_SOURCE_DIR}
		${CMAKE_SOURCE_DIR}/core
		${CMAKE_SOURCE_DIR}/drivers
		${CMAKE_SOURCE_DIR}/services
		${CMAKE_SOURCE_DIR}/config
)
