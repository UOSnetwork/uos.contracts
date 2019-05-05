add_contract(uos.dao uos.dao uos.dao.cpp)

target_include_directories(uos.dao
        PUBLIC
        )

set_target_properties(uos.dao
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
