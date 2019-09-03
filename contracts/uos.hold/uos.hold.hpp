#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
//#include <eosiolib/print.hpp>
//#include <eosiolib/crypto.h>
//#include <eosiolib/public_key.hpp>
//#include <eosiolib/types.hpp>
#include <eosio/asset.hpp>
//#include <eosiolib/symbol.hpp>
#include <eosio/singleton.hpp>
//#include <eosio.token/eosio.token.hpp>
//#include <string>
//#include <cstdint>
//#include <vector>


namespace UOS {
    using namespace eosio;
    using std::string;

    class [[eosio::contract("uos.hold")]]uos_hold : public eosio::contract {
    public:
        uos_hold(name receiver, name code, datastream<const char*> ds)
                : contract(receiver, code, ds) , _limits(code,receiver.value){
        }

        [[eosio::action]]
        void settime(int64_t begin, int64_t end);

        [[eosio::action]]
        void transfer(name from, name to, asset quantity, string memo);

        [[eosio::action]]
        void withdraw(name acc_name);

    private:

        struct  [[eosio::table("limits")]]
        time_limits{
            uint64_t begin;
            uint64_t end;
        };

        struct  [[eosio::table]]
        balance_entry {
            name acc_name;
            uint64_t deposit;
            uint64_t withdrawal;

            uint64_t primary_key() const { return acc_name.value; }

            EOSLIB_SERIALIZE(balance_entry, (acc_name)(deposit)(withdrawal))
        };

        typedef eosio::singleton <"limits"_n, time_limits> time_limits_singleton;

        typedef multi_index <"balance"_n, balance_entry> balance_table;

        time_limits_singleton _limits;

    };
}