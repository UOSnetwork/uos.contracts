add_contract(uos.register uos.register uos.register.cpp)

target_include_directories(uos.accinfo
        PUBLIC
        )

set_target_properties(uos.register
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")