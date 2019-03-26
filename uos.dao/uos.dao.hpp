

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

    class uos_dao : public eosio::contract {
    public:
        uos_dao(account_name self) : contract(self) {}

        /// @abi action
        void setparam(string name, string value);

    private:
        ///@abi table receipt
        struct param {
            uint64_t id;
            string name;
            string value;

            uint64_t primary_key() const { return id; }

            EOSLIB_SERIALIZE(param, (id)(name)(value))
        };

        typedef multi_index <N(params), param> params_table;

    };
}


