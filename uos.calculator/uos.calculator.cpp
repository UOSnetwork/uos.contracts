
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

        eosio_assert(value.symbol.name()==_state.get().base_asset.symbol.name(),"Asset symbol is incompatible with used in contract");

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
        eosio_assert(is_account(_state.get().fund_name),(std::string("create uos.stake first ")+ name{_state.get().fund_name}.to_string()).c_str());
        INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {acc, N(active)},
                                                      { acc, _state.get().fund_name, value, std::string("stake tokens") } );

    }

    void uos_calculator::refund(const account_name acc) {
        require_auth(acc);
        voters_table voters(_self,acc);
        auto itr=voters.find(acc);
        eosio_assert(itr != voters.end(), "there is nothing to refund here");
        //todo check timeout

        eosio_assert(is_account(_state.get().fund_name),"create uos.stake first");


        INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {_state.get().fund_name, N(active)},
                                                      { _state.get().fund_name, itr->owner, itr->stake, std::string("unstake tokens") } );

        voters.erase(itr);

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

    void uos_calculator::votecalc(const account_name acc, std::vector<account_name> calcs) {
        require_auth(acc);

        for(auto item : calcs){
            eosio_assert(is_account(item),(string("account not found: ") + (name{item}).to_string()).c_str());
            eosio_assert(check_calc(item),(string("account is not registered or not active: ") + (name{item}).to_string()).c_str());
        }
        //todo: add votes to accounts
    }

    void uos_calculator::setasset(const eosio::asset value) {
        require_auth(_self);
        eosio_assert((_state.get().base_asset.symbol)!=(value.symbol),"Nothing to change");
        //todo check if there is someone's stake, then..
    }

}
