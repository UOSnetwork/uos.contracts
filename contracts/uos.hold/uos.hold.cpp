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

    void uos_hold::deposit(name acc_name, eosio::asset amount) {
           print("DEPOSIT","\n");
           print("ACC_NAME ", name{acc_name}, "\n");
           print("AMOUNT ");amount.print();print("\n");
//         require_auth(_self);

// //        print("EXTERNAL ID ", external_id, '\n');
// //        print("AIRDROP ID ", airdrop_id, '\n');
// //        print("AMOUNT ", amount, '\n');
// //        print("ACC NAME ", name{acc_name}, '\n');
// //        print("SYMBOL ", symbol, '\n');

//         receipt_table r_table(_self, _self.value);

// //        print("CHECKING FOR EXTERNAL_ID\n");
//         auto ei_index = r_table.get_index<"external.id"_n>();
//         auto ei_itr = ei_index.find(external_id);
//         eosio_assert(ei_itr == ei_index.end(), "Already have the receipt with the same external_id");

// //        print("CHECKING FOR ACC_NAME+AIRDROP_ID\n");
//         auto an_index = r_table.get_index<"acc.name"_n>();
//         for(auto an_itr = an_index.find(acc_name.value); an_itr != an_index.end() && an_itr->acc_name == acc_name; an_itr++){
// //            print(an_itr->id, " ", an_itr->external_id, " ", an_itr->airdrop_id, " ", name{an_itr->acc_name}, "\n");
//             eosio_assert(an_itr->airdrop_id != airdrop_id, "Already have the receipt with the same acc_name and airdrop_id");
//         }

// //        print("SENDING THE TOKENS\n");
//         eosio::asset ast(amount, eosio::symbol(_symbol,4));
// //        ast.symbol.print();print("\n");
// //        ast.print();print("\n");
//         INLINE_ACTION_SENDER(eosio::token, transfer)( "eosio.token"_n, {_self, "active"_n},
//                                                       { _self, acc_name, ast, std::string("airdrop") } );

// //        print("ADDING THE RECEIPT\n");
//         r_table.emplace(_self, [&](rec_entry &rec) {
//             rec.id = r_table.available_primary_key();
//             rec.external_id = external_id;
//             rec.airdrop_id = airdrop_id;
//             rec.amount = amount;
//             rec.acc_name = acc_name;
//             rec.symbol = _symbol;
//         });
    }

    void uos_hold::withdraw(name acc_name) {
           print("WITHDRAW","\n");
           print("ACC_NAME ", name{acc_name}, "\n");
    }

    EOSIO_DISPATCH( uos_hold, (settime)(deposit)(withdraw))
}
