#include "uos.hold.hpp"
#include <eosio.token/eosio.token.hpp>


namespace UOS {

    void uos_hold::settime(int64_t begin, int64_t end) {
           print("SETTIME","\n");
           print("BEGIN ", begin, "\n");
           print("END ", end, "\n");

           //check self-authentication
           require_auth(_self);

           //check if begin is before end
           eosio_assert(begin < end, "begin must be less than end");

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
        print("MEMO ", memo, "\n");
    }

    void uos_hold::withdraw(name acc_name) {
           print("WITHDRAWW","\n");
           print("ACC_NAME ", name{acc_name}, "\n");
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
    