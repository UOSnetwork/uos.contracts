#pragma once

#include <eosiolib/eosio.hpp>
//#include <eosiolib/print.hpp>
//#include <eosiolib/crypto.h>
//#include <eosiolib/public_key.hpp>
//#include <eosiolib/types.hpp>
//#include <eosiolib/asset.hpp>
//#include <eosiolib/symbol.hpp>
//#include <eosiolib/singleton.hpp>
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
                : contract(receiver, code, ds){}

        [[eosio::action]]
        void send(uint64_t external_id,
                        uint64_t airdrop_id,
                        uint64_t amount,
                        name acc_name,
                        string symbol);
    private:

        // struct  [[eosio::table]]
        // rec_entry {
        //     uint64_t id;
        //     uint64_t external_id;
        //     uint64_t airdrop_id;
        //     uint64_t amount;
        //     name acc_name;
        //     string symbol;

        //     uint64_t primary_key() const { return id; }
        //     uint64_t by_external_id() const { return external_id; }
        //     uint64_t by_acc_name() const { return acc_name.value; }

        //     EOSLIB_SERIALIZE(rec_entry, (id)(external_id)(airdrop_id)(amount)(acc_name)(symbol))
        // };

        // typedef multi_index <"receipt"_n, rec_entry
        //         , indexed_by<"external.id"_n, const_mem_fun<rec_entry, uint_fast64_t , &rec_entry::by_external_id>>
        //         , indexed_by<"acc.name"_n, const_mem_fun<rec_entry, uint_fast64_t , &rec_entry::by_acc_name>>
        // > receipt_table;

    };
}


