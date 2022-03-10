cmf_generate_cpp(header cpp skvbc::messages skvbc_msgs.cmf)
add_library(cmf_skvbc_msgs ${cpp})
set_target_properties(cmf_skvbc_msgs PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(cmf_skvbc_msgs PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
