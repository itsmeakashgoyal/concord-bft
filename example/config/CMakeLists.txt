project (test_comm_config_lib LANGUAGES CXX)

add_library(test_comm_config_lib
        test_comm_config.cpp
        )
target_include_directories(test_comm_config_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${bftengine_SOURCE_DIR}/include
        ${bftengine_SOURCE_DIR}/include/bftengine
        ${threshsign_SOURCE_DIR}/include
        ${concord_bft_tools_SOURCE_DIR})

target_link_libraries(test_comm_config_lib PRIVATE bftcommunication)
if (USE_S3_OBJECT_STORE)
target_link_libraries(test_comm_config_lib PRIVATE concordbft_storage)
endif()