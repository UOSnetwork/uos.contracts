
#include <eosio.token/eosio.token.hpp>
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

    void uos_calculator::stake(const account_name acc, const eosio::asset value) {
        require_auth(acc);

        //todo paste here  "comparing symbol of stacked value with symbol of contract asset"
        voters_table voters(_self,acc);
        auto itr=voters.find(acc);
        if(itr == voters.end()){
            voters.emplace(acc,[&](voter_info& a){
               a.stake = value;
               a.owner = acc;
            });
        }else{
            voters.modify(itr,acc,[&](voter_info& a){
               a.stake+=value;
            });
        }
        eosio_assert(is_account(N(uos.stake)),"create uos.stake first");
        INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {acc, N(active)},
                                                      { acc, N(uos.stake), value, std::string("stake tokens") } );

    }

    void uos_calculator::refund(const account_name acc) {
        require_auth(acc);
        voters_table voters(_self,acc);
        auto itr=voters.find(acc);
        eosio_assert(itr != voters.end(), "there is nothing to refund here");
        //todo check timeout

        eosio_assert(is_account(N(uos.stake)),"create uos.stake first");


        INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {N(uos.stake), N(active)},
                                                      { N(uos.stake), itr->owner, itr->stake, std::string("unstake tokens") } );

        voters.erase(itr);

    }

}
