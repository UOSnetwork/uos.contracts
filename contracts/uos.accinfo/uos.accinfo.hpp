

#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <string>
#include <cstdint>
#include <vector>


namespace UOS {
    using namespace eosio;
    using std::string;

    class [[eosio::contract("uos.accinfo")]]uos_accinfo : public eosio::contract {
    public:
        uos_accinfo(name receiver, name code, datastream<const char*> ds)
                : contract(receiver, code, ds){}

        [[eosio::action]]
        void setprofile(name acc, string profile_json);
    private:


        struct [[eosio::table]]
        acc_profile {
            name acc;
            string profile_json;

            uint64_t primary_key() const { return acc.value; }

            EOSLIB_SERIALIZE(acc_profile, (acc)(profile_json))
        };

        typedef multi_index <"accprofile"_n, acc_profile> accprofile_table;
    };
}


