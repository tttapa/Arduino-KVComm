# - Config file for the KVComm package

# Compute paths
get_filename_component(KVCOMM_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# Our library dependencies (contains definitions for IMPORTED targets)
if(NOT TARGET kvcomm)
  include("${KVCOMM_CMAKE_DIR}/KVCommTargets.cmake")
endif()
