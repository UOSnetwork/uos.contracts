#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/public_key.hpp>
#include <eosiolib/types.h>
#include <eosiolib/asset.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/chain.h>

namespace UOS {
    using namespace eosio;
    using std::string;

    class uos_calculation_stats : public eosio::contract {
    public:
        uos_calculation_stats(account_name self) : contract(self) {}

        /// @abi action
        void setstats(uint64_t block_num, string network_activity, string emission, string total_emission);
    private:

        ///@abi table calcstats
        struct calc_stats {
            uint64_t block_num;
            string network_activity;
            string emission;
            string total_emission;
            uint64_t primary_key() const { return block_num; }
            uint64_t reverse_key() const { return ~block_num; }

            EOSLIB_SERIALIZE(calc_stats, (block_num)(network_activity)(emission)(total_emission))
        };

        typedef multi_index <N(calcstats), calc_stats,indexed_by<N(reverse_key),
                const_mem_fun<calc_stats, uint64_t , &calc_stats::reverse_key>>> calcstats_table;

    };
}


