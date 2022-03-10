project (comm_config_lib LANGUAGES CXX)

add_library(comm_config_lib
        CommunicationConfig.cpp
        )
target_include_directories(comm_config_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${bftengine_SOURCE_DIR}/include
        ${bftengine_SOURCE_DIR}/include/bftengine
        ${threshsign_SOURCE_DIR}/include
        ${concord_bft_tools_SOURCE_DIR})

target_link_libraries(comm_config_lib PRIVATE bftcommunication)

