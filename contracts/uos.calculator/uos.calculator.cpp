
#include <eosio.token/eosio.token.hpp>
#include <map>
#include "uos.calculator.hpp"

namespace UOS{

    void uos_calculator::regcalc(const name acc, const eosio::public_key& key, const string& url, uint16_t location) {
        require_auth(acc);
        calcs_table calcs(_self,_self.value);
        auto itr = calcs.find(acc.value);
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

    void uos_calculator::rmcalc(const name acc) {
        require_auth(_self);
        calcs_table calcs(_self,_self.value);
        auto itr = calcs.find(acc.value);
        if( itr != calcs.end() ){
            calcs.erase(itr);
        }
    }

    void uos_calculator::unregcalc(const name acc) {
        require_auth(acc);
        calcs_table calcs(_self,_self.value);
        auto itr = calcs.find(acc.value);
        if( itr != calcs.end() ){
            calcs.modify(itr,_self,[&](calc_info &item){
               item.deactivate();
            });
        }
    }

    void uos_calculator::stake(const name acc, const eosio::asset value) {
        require_auth(acc);

        eosio_assert(value.symbol==_state.get().base_asset.symbol,"Asset symbol is incompatible with used in contract");

        voters_table voters(_self,acc.value);
        auto itr=voters.find(acc.value);
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
        INLINE_ACTION_SENDER(eosio::token, transfer)( "eosio.token"_n, {acc, "active"_n},
                                                      { acc, _state.get().fund_name, value, std::string("stake tokens") } );

    }

    void uos_calculator::refund(const name acc) {
        require_auth(acc);
        voters_table voters(_self,acc.value);
        auto itr=voters.find(acc.value);
        eosio_assert(itr != voters.end(), "there is nothing to refund here");
        //todo check timeout

        eosio_assert(is_account(_state.get().fund_name),"create uos.stake first");

        //todo: take back votes from candidates first of all (unvote)

        INLINE_ACTION_SENDER(eosio::token, transfer)( "eosio.token"_n, {_state.get().fund_name, "active"_n},
                                                      { _state.get().fund_name, itr->owner, itr->stake, std::string("unstake tokens") } );


        voters.erase(itr);

    }

    void uos_calculator::iscalc(const name acc) {
        require_auth(acc);
        calcs_table calcs(_self,_self.value);
        auto itr = calcs.find(acc.value);
        eosio_assert(itr!=calcs.end(),(string("This account is not in calcs")+name{acc}.to_string()).c_str());
    }

    bool uos_calculator::check_calc(const name acc) {
        calcs_table calcs(_self,_self.value);
        auto itr = calcs.find(acc.value);
        if(itr==calcs.end())
            return false;
        return itr->is_active;
    }

    asset uos_calculator::get_stake(name voter) {
        voters_table voters(_self,voter.value);
        auto itr = voters.find(voter.value);
        if(itr==voters.end()){
            asset ret = _state.get().base_asset;
            ret.amount = 0;
            return ret;
        }
        return itr->stake;

    }

    void uos_calculator::votecalc(const name voter, std::vector<name> calcs) {
        require_auth(voter);
        voters_table voters(_self,voter.value);
        auto itr_voter = voters.find(voter.value);
        eosio_assert(itr_voter!=voters.end(),"Voter not found in table. Need to stake first");
        eosio_assert(itr_voter->stake_voted.amount==0,"unvote all calcs first");

        for(auto item : calcs){
            eosio_assert(is_account(item),(string("account not found: ") + (name{item}).to_string()).c_str());
            eosio_assert(check_calc(item),(string("account has not been registered or not active: ") + (name{item}).to_string()).c_str());
        }

        int64_t vote_for_each = get_stake(voter).amount/(int64_t )calcs.size();
        eosio_assert(vote_for_each>0, "you can not vote zero tokens for each candidate");
        calcs_table c_table(_self,_self.value);
        for(auto item : calcs){
            auto itr = c_table.find(item.value);
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

    bool uos_calculator::unvote_vector(voters_table &voters, voters_table::const_iterator &itr, const name &voter, std::vector<name> &calcs_to_unvote) {
        calcs_table c_table(_self,_self.value);
        voters.modify(itr,voter,[&](voter_info& vi){
            //get votes back
            std::vector<candidate_info> new_list;
            for(auto calc: vi.calcs){
                if(std::find(calcs_to_unvote.begin(),calcs_to_unvote.end(),calc.calc)!=calcs_to_unvote.end()){
                    auto citr = c_table.find(calc.owner.value);
                    //eosio_assert(citr!=c_table.end(),"Something wrong with data about calc");
                    if(citr==c_table.end())
                        return false; //todo
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

    void uos_calculator::unvote(const name voter, std::vector<name> calcs_to_unvote) {
        require_auth(voter);
        voters_table voters(_self,voter.value);
        auto itr = voters.find(voter.value);
        eosio_assert(itr!=voters.end(),"Voter not found");

        eosio_assert(unvote_vector(voters,itr,voter,calcs_to_unvote),"unvote vector error");


    }

    void uos_calculator::unvoteall(const name voter) {
        require_auth(voter);
        voters_table voters(_self,voter.value);
        auto itr = voters.find(voter.value);
        eosio_assert(itr!=voters.end(),"Voter not found");
        std::vector<name> calcs_list;
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
    

    void uos_calculator::withdrawal(name owner) {
        require_auth(owner);
        print("hello");
        accounts_table actable(_self,owner.value);
        auto act_itr = actable.find(owner.value);
        eosio_assert(act_itr!=actable.end(),"Reward not found");
        double reward = act_itr->account_sum;
        print(reward);
        asset val;
        val.symbol = symbol(CORE_SYMBOL,4);
        print(val);
        auto dotdigits = (uint64_t )val.symbol.precision();
        auto k = 1;
        for(uint64_t i=0;i<dotdigits;++i ){
            k*=10;
        }
        double temp = reward*k;

        val.amount = static_cast<int64_t >(temp);
        eosio_assert(val.amount > 0,"nothing to withdrawal");
        print(val);

        actable.modify(act_itr,owner,[&](account_info &account){
            account.account_sum-=double(val.amount)/k;
        });
        INLINE_ACTION_SENDER(eosio::token, issue) ( name{"eosio.token"}, {name{"eosio"},name{"active"}},{_self, val, std::string("issue tokens for account")} );
        INLINE_ACTION_SENDER(eosio::token, transfer) ( name{"eosio.token"}, {_self, name{"active"}} ,{ _self, owner, val, std::string("transfer issued tokens for account")} );
    }

    void uos_calculator::withdraw(name owner, double sum) {
        require_auth(owner);
        double fsum = double(sum);
        accounts_table actable(_self,owner.value);
        auto act_itr = actable.find(owner.value);
        eosio_assert(act_itr!=actable.end(),"Reward not found");
        if (fsum > act_itr->account_sum ){
            fsum = act_itr->account_sum;
        }
        double reward = fsum;
        asset val;
        val.symbol = symbol(CORE_SYMBOL,4);
        auto dotdigits = (uint64_t )val.symbol.precision();
        auto k = 1;
        for(uint64_t i=0;i<dotdigits;++i ){
            k*=10;
        }
        double temp = reward*k;
        val.amount = static_cast<int64_t >(temp);
        eosio_assert(val.amount > 0,"nothing to withdrawal");
        print(val);

        actable.modify(act_itr,owner,[&](account_info &account){
            account.account_sum-=double(val.amount)/k;
        });
        INLINE_ACTION_SENDER(eosio::token, issue) ( name{"eosio.token"}, {name{"eosio"},name{"active"}},{_self, val, std::string("issue tokens for account")} );
        INLINE_ACTION_SENDER(eosio::token, transfer) ( name{"eosio.token"}, {_self, name{"active"}} ,{ _self, owner, val, std::string("transfer issued tokens for account")} );
    }

    void uos_calculator::addsum(name issuer, name receiver, double sum, std::string message) {
        require_auth(issuer);
        print(message);
        eosio_assert(is_account(receiver),"error account name");
//        eosio_assert(is_issuer(issuer),"account is not registered as issuer");
        accounts_table actable(_self,receiver.value);
        auto act_itr = actable.find(receiver.value);
        if(act_itr==actable.end()){
            actable.emplace(receiver,[&](account_info &item){
                item.owner=receiver;
                item.account_sum=sum;
            });
        }else{
            actable.modify(act_itr,receiver,[&](account_info &item){
                item.account_sum+=sum;
            });
        }
    }

    void uos_calculator::regissuer(name issuer) {
        require_auth(issuer);
        eosio_assert(!is_issuer(issuer),"Already registered as issuer");
        issuers_table isstable(_self,_self.value);
        isstable.emplace(issuer,[&](issuer_info &item){
            item.issuer=issuer;
        });
    }

    bool uos_calculator::is_issuer(name acc) {
        issuers_table isstable(_self,_self.value);
        auto iss_itr = isstable.find(acc.value);
        return iss_itr!=isstable.end();
    }

    void uos_calculator::setrate(string name, string value) {
        require_auth(_self);
        checksum256 result;
        result = sha256((char *) name.c_str(), strlen(&name[0]));
        rateIndex rates(_self, _self.value);

        string name_acc = name;
        auto secondary_index = rates.get_index<"name.hash"_n>();
        auto itr = secondary_index.lower_bound(rate::get_hash(result));

        if (itr->acc_name == name_acc) {
//        secondary_index.erase(itr);//erase should be failed
            auto iter_rate = rates.find(itr->key);
            eosio_assert(iter_rate != rates.end(), "Rate is key not found");

            rates.modify(iter_rate, _self, [&](rate &item) {
                item.value = value;
            });
        } else {
            rates.emplace(_self, [&](rate &rate_item) {
                rate_item.key = rates.available_primary_key();
                rate_item.name_hash = result;
                rate_item.value = value;
                rate_item.acc_name = name_acc;
            });
        }

    }

    void uos_calculator::eraserate(uint64_t index) {
        require_auth(_self);
        rateIndex rates(_self, _self.value);
        auto iter = rates.find(index);
        //rates.erase(rates.get(index));
        if(iter != rates.end())
                rates.erase(iter);
    }

    void uos_calculator::erase(uint64_t number = 0) {
        require_auth(_self);
        rateIndex rates(_self, _self.value);
        if (number == 0) {
            for (; rates.begin() != rates.end();)
                rates.erase(rates.begin());
        } else {
            for (uint64_t i = 0; i < number; i++) {
                if (rates.begin() != rates.end()) {
                    rates.erase(rates.begin());
                } else {
                    break;
                }
            }

        }
    }

    void uos_calculator::setratetran(string name, string value) {
        require_auth(_self);
        checksum256 result;
        result = sha256((char *) name.c_str(), strlen(&name[0]));
        ratetrIndex ratestr(_self, _self.value);

        string name_acc = name;
        auto secondary_index = ratestr.get_index<name{"name_hash"}.value>();
        auto itr = secondary_index.lower_bound(rate::get_hash(result));

        if (itr->acc_name == name_acc) {
//        secondary_index.erase(itr);//erase should be failed
            auto iter_rate = ratestr.find(itr->key);
            eosio_assert(iter_rate != ratestr.end(), "Rate is key not found ");

            ratestr.modify(iter_rate, _self, [&](ratetr &item) {
                item.value = value;
            });
        } else {
            ratestr.emplace(_self, [&](ratetr &ratetr_item) {
                ratetr_item.key = ratestr.available_primary_key();
                ratetr_item.name_hash = result;
                ratetr_item.value = value;
                ratetr_item.acc_name = name_acc;
            });
        }

    }

    void uos_calculator::setallcalc(std::vector<name> accounts) {
        require_auth(_self);
        calcreg_table cr_table(_self, _self.value);

        //erase all registered calculators
        for(;cr_table.begin() != cr_table.end();)
            cr_table.erase(cr_table.begin());

        //register all account from the input
        for(auto itr : accounts)
        {
            print(name{itr}, "\n");
            if(cr_table.find(itr) == cr_table.end()) {
                cr_table.emplace(_self, [&](auto &calc_reg) {
                    calc_reg.owner = itr;
                });
            }
        }
    }

    void uos_calculator::reporthash(const name acc, string hash, uint64_t block_num, string memo) {
        require_auth(acc);

        print("reporthash  acc = ", name{acc}, " hash = ", hash, " block_num = ", (int)block_num, " memo = ", memo, "\n");

        calcreg_table cr_table(_self, _self.value);
        reports_table r_table(_self, _self.value);
        consensus_bl_table cb_table(_self, _self.value);

        //check acc to be registered as calculator
        auto itp_reg = cr_table.find(acc.value);
        eosio_assert(itp_reg != cr_table.end(), "account is not a registered calculator");

        //check for the report with the same acc + block_num
        auto ab_index = r_table.get_index<name{"acc_block"}.value>();
        auto ab_hash = calc_reports::get_acc_block_hash(acc, block_num);
        auto itr_rep = ab_index.find(ab_hash);
        eosio_assert(itr_rep == ab_index.end(), "hash already reported for this block");

        r_table.emplace(_self, [&](calc_reports &calc_rep) {
            calc_rep.key = r_table.available_primary_key();
            calc_rep.acc = acc;
            calc_rep.hash = hash;
            calc_rep.block_num = block_num;
            calc_rep.memo = memo;
        });

        //check if the consensus record already exists
        if(cb_table.find(block_num) != cb_table.end())
            return;

        //count the active calculator accounts
        int calc_count = 0;
        for(auto i = cr_table.begin(); i != cr_table.end(); i++)
            calc_count++;

        //count the reports with identical block_num+hash and find the leader
        int rep_count = 0;
        int leader_key = -1;
        name leader_name;
        auto bn_index = r_table.get_index<name{"block_num"}.value>();
        for(auto i = bn_index.find(block_num); i != bn_index.end() && i->block_num == block_num; i++)
        {
            if(i->hash != hash)
                continue;

            rep_count++;

            //we define the leader as an account, who reported the consensus hash first, i.e. who has the lowest key
            if(leader_key == -1 || (int)i->key < leader_key)
            {
                leader_key = (int)i->key;
                leader_name = i->acc;
            }
        }

        if(rep_count * 4 > calc_count * 3)
        {
            cb_table.emplace(_self, [&](cons_block &cb_item){
                cb_item.block_num = block_num;
                cb_item.hash = hash;
                cb_item.leader = leader_name;
            });
        }
    }

    EOSIO_ABI(uos_calculator,(regcalc)(rmcalc)(unregcalc)(iscalc)(stake)(refund)(votecalc)(setasset)(addsum)(regissuer)(withdrawal)(withdraw)(setrate)(eraserate)(erase)(setratetran)(setallcalc)(reporthash))
}
