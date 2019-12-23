
//#include <eosio.token/eosio.token.hpp>
#include <map>
#include "uos.calculator.hpp"

namespace UOS{

    void uos_calculator::withdrawal(name owner) {
        require_auth(owner);
        accounts_table actable(_self,owner.value);
        auto act_itr = actable.find(owner.value);
        check(act_itr!=actable.end(),"Reward not found");
        double reward = act_itr->account_sum;
        asset val;
        val.symbol = symbol(CORE_SYMBOL,4);
        auto dotdigits = (uint64_t )val.symbol.precision();
        auto k = 1;
        for(uint64_t i=0;i<dotdigits;++i ){
            k*=10;
        }
        double temp = reward*k;

        val.amount = static_cast<int64_t >(temp);
        check(val.amount > 0,"nothing to withdrawal");

        actable.modify(act_itr,owner,[&](account_info &account){
            account.account_sum-=double(val.amount)/k;
        });

        totals_table tottable(_self,_self.value);
        auto tot_itr = tottable.find(owner.value);
        if(tot_itr==tottable.end()){
            tottable.emplace(owner,[&](total_values &item){
                item.owner=owner;
                item.total_emission=val;
                item.total_withdrawal=val;
            });
        }else{
            tottable.modify(tot_itr,owner,[&](total_values &item){
                item.total_withdrawal+=val;
            });
        }

        action(
            permission_level{ name{"eosio"},name{"active"} },
            name{"eosio.token"}, name{"issue"},
            std::make_tuple(name{"eosio"}, val, std::string("issue tokens for account"))
        ).send();
        action(
            permission_level{ name{"eosio"}, name{"active"} },
            name{"eosio.token"}, name{"transfer"},
            std::make_tuple(name{"eosio"}, owner, val, std::string("transfer issued tokens for account"))
        ).send();
    }

    void uos_calculator::withdraw(name owner, double sum) {
        require_auth(owner);
        double fsum = double(sum);
        accounts_table actable(_self,owner.value);
        auto act_itr = actable.find(owner.value);
        check(act_itr!=actable.end(),"Reward not found");
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
        check(val.amount > 0,"nothing to withdrawal");

        actable.modify(act_itr,owner,[&](account_info &account){
            account.account_sum-=double(val.amount)/k;
        });

        asset ast;
        ast.symbol = symbol(CORE_SYMBOL,4);
        ast.amount = static_cast<int64_t>(act_itr->account_sum*10000);
        
        totals_table tottable(_self,_self.value);
        auto tot_itr = tottable.find(owner.value);
        if(tot_itr==tottable.end()){
            tottable.emplace(owner,[&](total_values &item){
                item.owner=owner;
                item.total_emission=ast;
                item.total_withdrawal=val;
            });
        }else{
            tottable.modify(tot_itr,owner,[&](total_values &item){
                item.total_withdrawal+=val;
            });
        }

        action(
            permission_level{ name{"eosio"},name{"active"} },
            name{"eosio.token"}, name{"issue"},
            std::make_tuple(name{"eosio"}, val, std::string("issue tokens for account"))
        ).send();
        action(
            permission_level{ name{"eosio"}, name{"active"} },
            name{"eosio.token"}, name{"transfer"},
            std::make_tuple(name{"eosio"}, owner, val, std::string("transfer issued tokens for account"))
        ).send();
    }

    void uos_calculator::addsum(name issuer, name receiver, double sum, std::string message) {
        require_auth(issuer);
        check(is_account(receiver),"error account name");
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

        asset ast;
        ast.symbol = symbol(CORE_SYMBOL,4);
        ast.amount = static_cast<int64_t>(sum*10000);
        
        totals_table tottable(_self,_self.value);
        auto tot_itr = tottable.find(receiver.value);
        if(tot_itr==tottable.end()){
            tottable.emplace(receiver,[&](total_values &item){
                item.owner=receiver;
                item.total_emission=ast;
                item.total_withdrawal=ast*0;
            });
        }else{
            tottable.modify(tot_itr,receiver,[&](total_values &item){
                item.total_emission+=ast;
            });
        }
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
            check(iter_rate != rates.end(), "Rate is key not found");

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
        auto secondary_index = ratestr.get_index< "name.hash"_n >();
        auto itr = secondary_index.lower_bound(rate::get_hash(result));

        if (itr->acc_name == name_acc) {
//        secondary_index.erase(itr);//erase should be failed
            auto iter_rate = ratestr.find(itr->key);
            check(iter_rate != ratestr.end(), "Rate is key not found ");

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
            print(itr, "\n");
            if(cr_table.find(itr.value) == cr_table.end()) {
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
        check(itp_reg != cr_table.end(), "account is not a registered calculator");

        //check for the report with the same acc + block_num
        auto ab_index = r_table.get_index<"acc.block"_n>();
        auto ab_hash = calc_reports::get_acc_block_hash(acc, block_num);
        auto itr_rep = ab_index.find(ab_hash);
        check(itr_rep == ab_index.end(), "hash already reported for this block");

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
        auto bn_index = r_table.get_index<"block.num"_n>();
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


    void uos_calculator::makecontorg(const name acc, string organization_id, string content_id, uint8_t content_type_id,
                                     string parent_content_id) {}
                                     
    EOSIO_DISPATCH(uos_calculator,(addsum)(withdrawal)(withdraw)(setrate)(eraserate)(erase)(setratetran)(setallcalc)(reporthash))
}
