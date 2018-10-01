
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

        //todo: take back votes from candidates first of all (unvote)

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

    asset uos_calculator::get_stake(account_name voter) {
        voters_table voters(_self,voter);
        auto itr = voters.find(voter);
        if(itr==voters.end()){
            asset ret = _state.get().base_asset;
            ret.amount = 0;
            return ret;
        }
        return itr->stake;

    }

    void uos_calculator::votecalc(const account_name voter, std::vector<account_name> calcs) {
        require_auth(voter);
        voters_table voters(_self,voter);
        auto itr_voter = voters.find(voter);
        eosio_assert(itr_voter!=voters.end(),"Voter not found in table. Need to stake first");
        eosio_assert(itr_voter->stake_voted.amount==0,"unvote all calcs first");

        for(auto item : calcs){
            eosio_assert(is_account(item),(string("account not found: ") + (name{item}).to_string()).c_str());
            eosio_assert(check_calc(item),(string("account has not been registered or not active: ") + (name{item}).to_string()).c_str());
        }

        int64_t vote_for_each = get_stake(voter).amount/(int64_t )calcs.size();
        eosio_assert(vote_for_each>0, "you can not vote zero tokens for each candidate");
        calcs_table c_table(_self,_self);
        for(auto item : calcs){
            auto itr = c_table.find(item);
            eosio_assert(itr!=c_table.end(),(string("account has not been registered or not active: ") + (name{item}).to_string()).c_str());
            c_table.modify(itr,_self,[&](calc_info& info){
               info.total_votes+= static_cast<uint64_t > (vote_for_each);
            });
        }
        auto votesum = vote_for_each*(int64_t)calcs.size();

        voters.modify(itr_voter,voter,[&](voter_info &vi){
            vi.stake_voted.symbol=vi.stake.symbol;
            vi.stake_voted.amount = votesum;
            for(auto calc : calcs){
                vi.calcs.push_back(candidate_info{calc,vote_for_each});
            }
//            vi.calcs = calcs;
        });
    }

    bool uos_calculator::unvote_vector(voters_table &voters, voters_table::const_iterator &itr, const account_name &voter, std::vector<account_name> &calcs_to_unvote) {
        calcs_table c_table(_self,_self);
        voters.modify(itr,voter,[&](voter_info& vi){
            //get votes back
            std::vector<candidate_info> new_list;
            for(auto calc: vi.calcs){
                if(std::find(calcs_to_unvote.begin(),calcs_to_unvote.end(),calc.calc)!=calcs_to_unvote.end()){
                    auto citr = c_table.find(calc.calc);
                    //eosio_assert(citr!=c_table.end(),"Something wrong with data about calc");
                    if(citr==c_table.end())
                        return false;
                    c_table.modify(citr,_self,[&](calc_info &ci){
                        ci.total_votes-= static_cast<uint64_t >(calc.bid);
                    });
                    vi.stake_voted.amount-=calc.bid;
                    calc.bid=0;
                }else{
                    new_list.push_back(calc);
                }
            }
            vi.calcs = new_list;
        });
        return true;

    }

    void uos_calculator::unvote(const account_name voter, std::vector<account_name> calcs_to_unvote) {
        require_auth(voter);
        voters_table voters(_self,voter);
        auto itr = voters.find(voter);
        eosio_assert(itr!=voters.end(),"Voter not found");

        eosio_assert(unvote_vector(voters,itr,voter,calcs_to_unvote),"unvote vector error");


    }

    void uos_calculator::unvoteall(const account_name voter) {
        require_auth(voter);
        voters_table voters(_self,voter);
        auto itr = voters.find(voter);
        eosio_assert(itr!=voters.end(),"Voter not found");
        std::vector<account_name> calcs_list;
        for(auto calc : itr->calcs){
            calcs_list.push_back(calc.calc);
        }
        eosio_assert(unvote_vector(voters,itr,voter,calcs_list),"unvote vector error");
        //unvote(voter,calcs_list);
    }

    void uos_calculator::setasset(const eosio::asset value) {
        require_auth(_self);
        eosio_assert((_state.get().base_asset.symbol)!=(value.symbol),"Nothing to change");
        //todo check if there is someone's stake, then..
    }

}
