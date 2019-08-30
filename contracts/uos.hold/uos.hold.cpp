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

        check(quantity.symbol.code().to_string() == "UOS", "only UOS core token can be accepted");

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
           print("WITHDRAWW","\n");
           print("ACC_NAME ", name{acc_name}, "\n");

           require_auth(acc_name);

           auto lim_begin = _limits.get().begin;
           print("BEGIN ", lim_begin, "\n");
           auto lim_end = _limits.get().end;
           print("END ", lim_end, "\n");
           
           check(0 < lim_begin && lim_begin < lim_end, "limits are not set properly");

           auto current_time = eosio::current_time_point().time_since_epoch()._count;
           print("CURRENT TIME ", current_time, "\n");
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
