add_contract(uos.hold_by_emission uos.hold_by_emission uos.hold_by_emission.cpp)

target_include_directories(uos.hold_by_emission
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../eosio.contracts/contracts/eosio.token/include)

set_target_properties(uos.hold_by_emission
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")