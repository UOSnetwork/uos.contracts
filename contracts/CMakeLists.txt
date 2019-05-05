cmake_minimum_required( VERSION 3.5 )

project(uos_contracts)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_subdirectory(eosio.fs)
add_subdirectory(uos.calculator)
add_subdirectory(uos.activity)
add_subdirectory(uos.airdrop)
add_subdirectory(uos.dao)