cmake_minimum_required( VERSION 3.5 )

project(uos_contracts)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_subdirectory(eosio.fs)
add_subdirectory(uos_calculation_stats)
add_subdirectory(uos.calculator)
add_subdirectory(uos.activity)
add_subdirectory(uos.airdrop)
add_subdirectory(uos.dao)
add_subdirectory(uos.system)
add_subdirectory(uos.accinfo)
add_subdirectory(uos.register)
add_subdirectory(uos.hold)
add_subdirectory(uos.hold_by_emission)
add_subdirectory(hello.merkle)