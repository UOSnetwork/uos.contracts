add_contract(uos.hold uos.hold uos.hold.cpp)

target_include_directories(uos.hold
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../eosio.contracts/contracts/eosio.token/include)

set_target_properties(uos.hold
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")