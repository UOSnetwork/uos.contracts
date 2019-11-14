#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <string>
#include <cstdint>
#include <vector>


namespace UOS {
    using namespace eosio;
    using std::string;

    class [[eosio::contract("uos_calculation_stats")]]uos_calculation_stats : public eosio::contract {
    public:
        uos_calculation_stats(name receiver, name code, datastream<const char*> ds)
                : contract(receiver, code, ds) {}

        [[eosio::action]]
        void setstats(uint64_t block_num, string network_activity, string emission, string total_emission);
    private:

        struct [[eosio::table]]
        calc_stats {
            uint64_t block_num;
            string network_activity;
            string emission;
            string total_emission;
            uint64_t primary_key() const { return block_num; }
            uint64_t reverse_key() const { return ~block_num; }

            EOSLIB_SERIALIZE(calc_stats, (block_num)(network_activity)(emission)(total_emission))
        };

        typedef multi_index <"calcstats"_n, calc_stats,indexed_by<"reversekey"_n,
                const_mem_fun<calc_stats, uint64_t , &calc_stats::reverse_key>>> calcstats_table;

    };
}


