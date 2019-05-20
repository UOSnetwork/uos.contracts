

#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/public_key.hpp>
//#include <eosiolib/types.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <string>
#include <cstdint>
#include <vector>


namespace UOS {
    using namespace eosio;
    using std::string;

    class [[eosio::contract("uos.dao")]]uos_dao : public eosio::contract {
    public:
        uos_dao(name receiver, name code, datastream<const char*> ds)
                : contract(receiver, code, ds){}

        [[eosio::action]]
        void setparam(string name, string value);

    private:
        struct  [[eosio::table]]
        param {
            uint64_t id;
            string name;
            string value;

            uint64_t primary_key() const { return id; }

            EOSLIB_SERIALIZE(param, (id)(name)(value))
        };

        typedef multi_index <"params"_n, param> params_table;

    };
}


