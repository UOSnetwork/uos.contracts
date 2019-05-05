add_contract(uos.calculator uos.calculator uos.calculator.cpp)

target_include_directories(uos.calculator
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../eosio.contracts/contracts/eosio.token/include)

set_target_properties(uos.calculator
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
