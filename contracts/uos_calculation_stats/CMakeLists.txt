add_contract(uos_calculation_stats uos_calculation_stats uos_calculation_stats.cpp)

target_include_directories(uos_calculation_stats
        PUBLIC
        )

set_target_properties(uos_calculation_stats
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")