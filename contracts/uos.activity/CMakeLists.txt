add_contract(uos.activity uos.activity uos.activity.cpp)

target_include_directories(uos.activity
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../eosio.contracts/contracts/eosio.token/include)

set_target_properties(uos.activity
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
