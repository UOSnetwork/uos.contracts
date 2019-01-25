

#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/public_key.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <string>
#include <cstdint>
#include <vector>


namespace UOS {
    using namespace eosio;
    using std::string;

    class uos_accinfo : public eosio::contract {
    public:
        uos_accinfo(account_name self) : contract(self) {}

        /// @abi action
        void setprofile(account_name acc, string profile_json);
    private:

        ///@abi table accprofile
        struct acc_profile {
            account_name acc;
            string profile_json;

            uint64_t primary_key() const { return acc; }

            EOSLIB_SERIALIZE(acc_profile, (acc)(profile_json))
        };

        typedef multi_index <N(accprofile), acc_profile> accprofile_table;
    };
}


