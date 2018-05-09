cmake_minimum_required(VERSION 3.10.2)

include(cmake/cotire.cmake)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Need this for static lib relocation
set(CMAKE_POSITION_INDEPENDENT_CODE YES)

# Setup hunter gate to auto grab our 3rd party dependencies
# (may be included by our super project so check HUNTER_WIKI)
if (NOT HUNTER_WIKI)
	include(cmake/HunterGate.cmake)
	HunterGate(
		URL "https://github.com/ruslo/hunter/archive/v0.20.39.tar.gz"
		SHA1 "b49c4b58e17c1473e822f8b21fcde92fee25791d"
	)
endif()

# Declare our project and version
project(DictosNet VERSION 0.1 LANGUAGES C CXX)

# boost stuff
hunter_add_package(Boost COMPONENTS context system)
find_package(Boost CONFIG REQUIRED)

# ssl stuff
hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

# nholmann json
hunter_add_package(nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)

# Our core file list
file(GLOB_RECURSE DictosNetSrc [LIST_DIRECTORIES false]
	${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)
file(GLOB_RECURSE DictosNetInc [LIST_DIRECTORIES false]
	${CMAKE_CURRENT_LIST_DIR}/inc/*.hpp
	${CMAKE_CURRENT_LIST_DIR}/inc/*.h
)

# Declare dictos::net lib
add_library(
	DictosNet INTERFACE
)

# Require c++17
target_compile_features(DictosNet INTERFACE cxx_std_17)

# Enable extended stacktrace info for boost, requires libdl
target_compile_definitions(DictosNet INTERFACE -DBOOST_STACKTRACE_USE_BACKTRACE -D_REENTRANT)

# Require clang libs
target_link_libraries(DictosNet INTERFACE
	DictosCore
	nlohmann_json
	OpenSSL::Crypto
	OpenSSL::SSL
)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(
	DictosNet INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# Setup a useful export file for inclusion in other projects
export(TARGETS DictosNet
	FILE DictosNetConfig.cmake
	EXPORT_LINK_INTERFACE_LIBRARIES
)

# Allow our package to be discoverable
export(PACKAGE DictosNet)

# Include the bin dir for config discovery
set(CONF_INCLUDE_DIRS "${PROJECT_LISTS_DIR}" "${PROJECT_BINARY_DIR}")

# Set install targets
install(TARGETS DictosNet EXPORT Net DESTINATION lib)
install(DIRECTORY inc/ DESTINATION include FILES_MATCHING PATTERN "*.h*")

# Load tests
enable_testing()
add_subdirectory(tests)
