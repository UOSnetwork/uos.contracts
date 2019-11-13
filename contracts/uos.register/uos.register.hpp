#pragma once

#include <eosio/eosio.hpp>
#include <string>
#include <cstdint>
#include <vector>

namespace UOS {
    using namespace eosio;
    using std::string;

    class [[eosio::contract("uos.register")]]uos_register : public eosio::contract {
    public:
        uos_register(name receiver, name code, datastream<const char*> ds)
                : contract(receiver, code, ds){}

        [[eosio::action]]
        void regname(name eos_account, name uos_account);
    private:


        struct [[eosio::table]]
        account_register {
            name eos_account;
            name uos_account;

            uint64_t primary_key() const { return eos_account.value; }

            EOSLIB_SERIALIZE(account_register, (eos_account)(uos_account))
        };

        typedef multi_index <"accregister"_n, account_register> accregister_table;
    };
}


