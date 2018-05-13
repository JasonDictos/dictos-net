# Our core file list
file(GLOB_RECURSE DictosNetTestDeps *.cpp *.h *.hpp)

add_executable(
	DictosNetTest
	${DictosNetTestDeps}
)

file(GLOB_RECURSE CoreDeps ../core/inc/*.h ../core/inc/*.hpp)
source_group("Core" FILES ${CoreDeps})
file(GLOB_RECURSE NetDeps ../net/inc/*.h ../net/inc/*.hpp)
source_group("Net" FILES ${NetDeps})

# Link to Catch2
hunter_add_package(Catch)
find_package(Catch2 CONFIG REQUIRED)

target_link_libraries(DictosNetTest DictosCore DictosNet Catch2::Catch)

add_test(DictosNetTest DictosNetTest)

include_directories(
	.
)

target_compile_features(DictosNetTest PUBLIC cxx_std_17)

# Precompiled header
set_target_properties(DictosNetTest PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "tests.hpp")
cotire(DictosNetTest)
