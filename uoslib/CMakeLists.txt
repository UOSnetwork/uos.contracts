add_wast_library(TARGET uoslib
        LIBRARIES libc libc++ eosiolib
        INCLUDE_FOLDERS "${STANDARD_INCLUDE_FOLDERS}"

        DESTINATION_FOLDER ${CMAKE_CURRENT_BINARY_DIR}
        )
