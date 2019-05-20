add_contract(hello.merkle hello.merkle hello_merkle.cpp)

target_include_directories(hello.merkle
        PUBLIC
        )

set_target_properties(hello.merkle
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")