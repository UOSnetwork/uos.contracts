add_contract(uos.airdrop uos.airdrop uos.airdrop.cpp)

target_include_directories(uos.airdrop
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../eosio.contracts/contracts/eosio.token/include)

set_target_properties(uos.airdrop
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
