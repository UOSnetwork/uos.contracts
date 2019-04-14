#file(GLOB ABI_FILES "*.abi")
#configure_file("${ABI_FILES}" "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)
#
#add_wast_executable(TARGET uos.calculator
#  INCLUDE_FOLDERS ${STANDARD_INCLUDE_FOLDERS}
#  LIBRARIES libc libc++ eosiolib eosio.token
#  DESTINATION_FOLDER ${CMAKE_CURRENT_BINARY_DIR}
#)
add_contract(uos.calculator uos.calculator uos.calculator.cpp)

target_include_directories(uos.calculator
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../eosio.contracts/contracts/eosio.token/include)

set_target_properties(uos.calculator
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
