add_contract(eosio.fs eosio.fs ${CMAKE_CURRENT_SOURCE_DIR}/eosio.fs.cpp)

target_include_directories(eosio.fs
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/
   ${CMAKE_CURRENT_SOURCE_DIR}/../../eosio.token/include)

set_target_properties(eosio.fs
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
