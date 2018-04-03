

# Set position independent code for our hunter builds
message("Hunter boost version: ${HUNTER_Boost_VERSION}")
hunter_config(Boost VERSION ${HUNTER_Boost_VERSION} CMAKE_ARGS CMAKE_CXX_FLAGS=-fPIC)
