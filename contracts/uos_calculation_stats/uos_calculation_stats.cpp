#include "uos_calculation_stats.hpp"

namespace UOS {

    void uos_calculation_stats::setstats(uint64_t block_num, string network_activity,string emission,string total_emission) {


//        auto s = transaction_size();
//        char buf[s];
//        auto size = read_transaction(buf, s);
//        eosio::transaction trx = eosio::unpack<eosio::transaction>(buf, s);
//        eosio_assert(block_num < trx.ref_block_prefix,"Block not exists");
//        print(trx.ref_block_prefix);
//        auto ref_bl_num = trx.ref_block_num;
//        printf("ref: %hu\n", ref_bl_num);

         //TODO: check block_number
        calcstats_table stats(_self, _self.value);
        auto itr = stats.find(block_num);
        if (itr != stats.end()) {
            stats.modify(itr,_self, [&](calc_stats &item) {
                item.network_activity = network_activity;
                item.emission = emission;
                item.total_emission = total_emission;
            });
        } else {
            stats.emplace(_self, [&](calc_stats &item) {
                item.block_num = block_num;
                item.network_activity = network_activity;
                item.emission = emission;
                item.total_emission = total_emission;

            });
        }
    }
}
