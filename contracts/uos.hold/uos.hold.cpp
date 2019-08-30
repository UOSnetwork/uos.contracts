#include "uos.hold.hpp"
//#include <eosio.token/eosio.token.hpp>


namespace UOS {

    void uos_hold::settime(int64_t begin, int64_t end) {
           print("SETTIME","\n");
           print("BEGIN ", begin, "\n");
           print("END ", end, "\n");

           //check self-authentication
           require_auth(_self);

           //check if begin is before end
           check(begin < end, "begin must be less than end");

           //check if the limits are already set
           if(_limits.get().begin != 0 || _limits.get().end != 0){
               print("LIMITS ARE SET ALREADY!!! TODO THROW AN EXCEPTION\n");
               print("BEGIN ", _limits.get().begin, "\n");
               print("END ", _limits.get().begin, "\n");
           }

           //set values
           time_limits temp;
           temp.begin = begin;
           temp.end = end;
           _limits.set(temp,_self);

    }

    void uos_hold::transfer(name from, name to, asset quantity, string memo) {
        print("TRANSFER\n");
        print("FROM ", name{from}, "\n");
        print("TO ", name{to}, "\n");
        print("QUANTITY ", quantity.to_string(), "\n");
        print("AMOUNT ", quantity.amount, "\n");
        print("SYMBOL CODE ", quantity.symbol.code().to_string(), "\n");
        print("MEMO ", memo, "\n");

        if(from == _self) {
            print("IGNORE OUTGOING TRANSFER\n");
            return;
        }

        check(quantity.symbol.code().to_string() == "UOS", "only UOS core token can be accepted");
        check(is_account(name{memo}), "must be an existing account name in memo");

        balance_table bals(_self,_self.value);
        auto acc_name = eosio::name(memo);
        auto itr = bals.find(acc_name.value);
        if(itr != bals.end()) {
            print("FOUND!!!!!\n");
            bals.modify(itr,_self, [&] (balance_entry &item){
                item.acc_name=acc_name;
                item.deposit=itr->deposit + quantity.amount;
                item.withdrawal=itr->withdrawal;
            });
            print("AND MODIFIED\n");
        }
        else {
            print("NOT FOUND!!!!!\n");
            bals.emplace(_self,[&] (balance_entry &item){
                item.acc_name=acc_name;
                item.deposit=quantity.amount;
                item.withdrawal=0;
            });
            print("AND ADDED\n");
        }
    }

    void uos_hold::withdraw(name acc_name) {
        print("WITHDRAW","\n");
        print("ACC_NAME ", name{acc_name}, "\n");
        
        require_auth(acc_name);
        
        auto lim_begin = _limits.get().begin;
        print("BEGIN ", lim_begin, "\n");
        auto lim_end = _limits.get().end;
        print("END ", lim_end, "\n");
        
        check(0 < lim_begin && lim_begin < lim_end, "limits are not set properly");
        
        balance_table bals(_self,_self.value);
        auto itr = bals.find(acc_name.value);
        check(itr != bals.end(), "balance record not found");

        auto current_time = eosio::current_time_point().time_since_epoch()._count;
        print("CURRENT TIME ", current_time, "\n");

        check(current_time > lim_begin, "withdrawal period not started yet");

        uint64_t withdraw_limit = 0;
        if(current_time > lim_end) {
            print("FULL DEPOSIT \n");
            withdraw_limit = itr->deposit;
        } else {
            print("SOME PART OF DEPOSIT \n");
            withdraw_limit = (uint64_t)((float)itr->deposit
                                      * (float)(current_time - lim_begin)
                                      / (float)(lim_end - lim_begin));
        }

        print("DEPOSIT ", itr->deposit, "\n");
        print("WITHDRAWAL ", itr->withdrawal, "\n");
        print("WITHDRAW LIMIT ", withdraw_limit, "\n");

        check(itr->withdrawal < withdraw_limit, "nothing to withdraw");

        uint64_t current_withdrawal = withdraw_limit - itr->withdrawal;
        print("CURRENT WITHDRAWAL ", current_withdrawal, "\n");
        
        //send current_withdrawal tokens to account
        asset ast(current_withdrawal, symbol("UOS",4));
        action(
            permission_level{ _self, name{"active"} },
            name{"eosio.token"}, name{"transfer"},
            std::make_tuple(_self, acc_name, ast, string("some memo here"))
        ).send();
        
        //increase withdrawal value
        bals.modify(itr,_self, [&] (balance_entry &item){
            item.acc_name=itr->acc_name;
            item.deposit=itr->deposit;
            item.withdrawal=itr->withdrawal + current_withdrawal;
        });        
    }
    
    extern "C" {
    void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        uos_hold _uos_hold(name(receiver));
        if(code==receiver && action== name("settime").value) {
            execute_action(name(receiver), name(code), &uos_hold::settime );
        }
        else if(code==receiver && action== name("withdraw").value) {
            execute_action(name(receiver), name(code), &uos_hold::withdraw );
        }
        else if(code==name("eosio.token").value && action== name("transfer").value) {
        execute_action(name(receiver), name(code), &uos_hold::transfer );
        }
    }
    }
}
