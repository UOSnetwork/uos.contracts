#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

namespace UOS {
    using namespace eosio;
    using std::string;

    class [[eosio::contract("uos.hold_by_emission")]]uos_hold_by_emission : public eosio::contract {
    public:
        typedef std::vector<checksum256> checksum_pair;

        uos_hold_by_emission(name receiver, name code, datastream<const char*> ds)
                : contract(receiver, code, ds) , _params(code,receiver.value){
        }

        [[eosio::action]]
        void setparams(int64_t time_begin, int64_t time_end, float multiplier, name emission_contract);

        [[eosio::action]]
        void transfer(name from, name to, asset quantity, string memo);

        [[eosio::action]]
        void withdraw(name acc_name);

    private:

        struct  [[eosio::table("params")]]
        parameters{
            uint64_t time_begin;
            uint64_t time_end;

            float multiplier;

            name emission_contract;
        };

        struct  [[eosio::table]]
        balance_entry {
            name acc_name;
            uint64_t deposit;
            uint64_t withdrawal;

            uint64_t primary_key() const { return acc_name.value; }

            EOSLIB_SERIALIZE(balance_entry, (acc_name)(deposit)(withdrawal))
        };

        // the values we get from the uos emission contract
        struct  //[[eosio::table]]
        total_values{
            name owner;
            asset total_emission;
            asset total_withdrawal;

            uint64_t  primary_key() const {return owner.value;}

            //EOSLIB_SERIALIZE(total_values, (owner)(total_emission)(total_withdrawal))
        };

        typedef eosio::singleton <"params"_n, parameters> parameters_singleton;

        typedef multi_index <"balance"_n, balance_entry> balance_table;
        
        // the table from the uos emission contract
        typedef multi_index <"totals"_n,total_values> totals_table;

        parameters_singleton _params;
    };
}