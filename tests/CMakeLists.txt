# Our core file list
file(GLOB_RECURSE NetTestDeps *.cpp *.h *.hpp)

add_executable(
	NetTest
	${NetTestDeps}
)

file(GLOB_RECURSE CoreDeps ../core/inc/*.h ../core/inc/*.hpp)
source_group("Core" FILES ${CoreDeps})
file(GLOB_RECURSE NetDeps ../net/inc/*.h ../net/inc/*.hpp)
source_group("Net" FILES ${NetDeps})

# Link to Catch2
hunter_add_package(Catch)
find_package(Catch2 CONFIG REQUIRED)

target_link_libraries(NetTest DictosCore DictosNet Catch)

add_test(NetTest NetTest)

include_directories(
	.
)

target_compile_features(NetTest PUBLIC cxx_std_17)

# Precompiled header
set_target_properties(NetTest PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "tests.hpp")
cotire(NetTest)
