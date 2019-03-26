

#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/public_key.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <eosio.token/eosio.token.hpp>
#include <string>
#include <cstdint>
#include <vector>


namespace UOS {
    using namespace eosio;
    using std::string;

    class uos_airdrop : public eosio::contract {
    public:
        uos_airdrop(account_name self) : contract(self) {}

        /// @abi action
        void send(uint64_t external_id,
                        uint64_t airdrop_id,
                        uint64_t amount,
                        account_name acc_name,
                        string symbol);
    private:

        ///@abi table receipt
        struct rec_entry {
            uint64_t id;
            uint64_t external_id;
            uint64_t airdrop_id;
            uint64_t amount;
            account_name acc_name;
            string symbol;

            uint64_t primary_key() const { return id; }
            uint64_t by_external_id() const { return external_id; }
            uint64_t by_acc_name() const { return acc_name; }

            EOSLIB_SERIALIZE(rec_entry, (id)(external_id)(airdrop_id)(amount)(acc_name)(symbol))
        };

        typedef multi_index <N(receipt), rec_entry
                , indexed_by<N(external_id), const_mem_fun<rec_entry, uint_fast64_t , &rec_entry::by_external_id>>
                , indexed_by<N(acc_name), const_mem_fun<rec_entry, uint_fast64_t , &rec_entry::by_acc_name>>
        > receipt_table;

    };
}


