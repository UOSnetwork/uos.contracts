add_contract(uos.accinfo uos.accinfo uos.accinfo.cpp)

target_include_directories(uos.accinfo
        PUBLIC
        )

set_target_properties(uos.accinfo
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")