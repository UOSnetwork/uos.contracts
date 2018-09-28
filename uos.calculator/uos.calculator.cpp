
#include "uos.calculator.hpp"

namespace UOS{

    void uos_calculator::regcalc(const account_name acc, const eosio::public_key& key, const string& url, uint16_t location) {
        require_auth(acc);
        calcs_table calcs(_self,_self);
        auto itr = calcs.find(acc);
        if( itr != calcs.end() ){
            calcs.modify(itr,_self, [&] (calc_info &item){
                item.location=location;
                item.url=url;
                item.is_active=true;
                item.calc_key=key;
            });
        }
        else{
            calcs.emplace(_self,[&] (calc_info &item){
                item.location=location;
                item.url=url;
                item.is_active=true;
                item.calc_key=key;
                item.owner=acc;

            });
        }
    }

    void uos_calculator::rmcalc(const account_name acc) {
        require_auth(_self);
        calcs_table calcs(_self,_self);
        auto itr = calcs.find(acc);
        if( itr != calcs.end() ){
            calcs.erase(itr);
        }
    }

    void uos_calculator::unregcalc(const account_name acc) {
        require_auth(acc);
        calcs_table calcs(_self,_self);
        auto itr = calcs.find(acc);
        if( itr != calcs.end() ){
            calcs.modify(itr,_self,[&](calc_info &item){
               item.deactivate();
            });
        }
    }

    void uos_calculator::iscalc(const account_name acc) {
        require_auth(acc);
        calcs_table calcs(_self,_self);
        auto itr = calcs.find(acc);
        eosio_assert(itr!=calcs.end(),(string("This account is not in calcs")+name{acc}.to_string()).c_str());
    }

    bool uos_calculator::check_calc(const account_name acc) {
        calcs_table calcs(_self,_self);
        auto itr = calcs.find(acc);
        if(itr==calcs.end())
            return false;
        return itr->is_active;
    }

    void uos_calculator::setallcalc(std::vector<account_name> accounts) {
        require_auth(_self);
        calcreg_table cr_table(_self, _self);

        //erase all registered calculators
        for(;cr_table.begin() != cr_table.end();)
            cr_table.erase(cr_table.begin());

        //register all account from the input
        for(auto itr = accounts.begin(); itr != accounts.end(); itr++)
        {
            print(name{*itr}, "\n");
            if(cr_table.find(*itr) == cr_table.end()) {
                cr_table.emplace(_self, [&](auto &calc_reg) {
                    calc_reg.owner = *itr;
                });
            }
        }
    }

    void uos_calculator::reporthash(const account_name acc, string hash, uint64_t block_num, string memo) {
        require_auth(acc);
        print("reporthash acc = ", name{acc}, " hash = ", hash, " block_num = ", (int)block_num, " memo = ", memo, "/n");

        calcreg_table cr_table(_self, _self);
        reports_table r_table(_self, _self);

        //check acc to be registered as calculator
        auto itp_reg = cr_table.find(name{acc});
        eosio_assert(itp_reg != cr_table.end(), "account is not a registered calculator");

        //check for the report with the same acc + block_num
        auto ab_index = r_table.get_index<N(acc_block)>();
        auto ab_hash = calc_reports::get_acc_block_hash(acc, block_num);
        auto itr_rep = ab_index.find(ab_hash);
        eosio_assert(itr_rep == ab_index.end(), "hash already reported for this block");

        r_table.emplace(_self, [&](auto &calc_rep) {
            calc_rep.key = r_table.available_primary_key();
            calc_rep.acc = acc;
            calc_rep.hash = hash;
            calc_rep.block_num = block_num;
            calc_rep.memo = memo;
        });
    }

}
