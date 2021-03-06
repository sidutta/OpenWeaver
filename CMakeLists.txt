cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(marlin VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_subdirectory(vendor EXCLUDE_FROM_ALL)

enable_testing()
add_custom_target(docs
	COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Generating API documentation with Doxygen"
	VERBATIM
)

add_subdirectory(core)
add_subdirectory(asyncio)
add_subdirectory(simulator)
add_subdirectory(beacon)
add_subdirectory(stream)
add_subdirectory(lpf)
add_subdirectory(pubsub)
add_subdirectory(multicastsdk)
add_subdirectory(relay)
add_subdirectory(goldfish)
add_subdirectory(rlpx)
add_subdirectory(integrations/eth)
