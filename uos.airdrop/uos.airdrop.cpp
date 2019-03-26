#include "uos.airdrop.hpp"
#include <eosio.token/eosio.token.hpp>


namespace UOS {
    void uos_airdrop::send(uint64_t external_id,
                                 uint64_t airdrop_id,
                                 uint64_t amount,
                                 account_name acc_name,
                                 std::string symbol) {
        print("EXTERNAL ID ", external_id, '\n');
        print("AIRDROP ID ", airdrop_id, '\n');
        print("AMOUNT ", amount, '\n');
        print("ACC NAME ", name{acc_name}, '\n');
        print("SYMBOL ", symbol, '\n');

        receipt_table r_table(_self, _self);

        //check for external_id
        print("CHECKING FOR EXTERNAL_ID\n");
        auto ei_index = r_table.get_index<N(external_id)>();
        auto ei_itr = ei_index.find(external_id);
        eosio_assert(ei_itr == ei_index.end(), "Already have the receipt with the same external_id");

        //check for acc_name+airdrop_id
        print("CHECKING FOR ACC_NAME+AIRDROP_ID\n");
        auto an_index = r_table.get_index<N(acc_name)>();
        for(auto an_itr = an_index.find(acc_name); an_itr != an_index.end() && an_itr->acc_name == acc_name; an_itr++){
            print(an_itr->id, " ", an_itr->external_id, " ", an_itr->airdrop_id, " ", name{an_itr->acc_name}, "\n");
            eosio_assert(an_itr->airdrop_id != airdrop_id, "Already have the receipt with the same acc_name and airdrop_id");
        }

        print("SENDING THE TOKENS\n");
        eosio::asset ast(amount, eosio::symbol_type(eosio::string_to_symbol(4,"UOSTEST")));
        //print("ASSET SYMBOL ", name{ast.symbol.name()}, "\n");
        ast.symbol.print();
        print("\n");
        //INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {_self, N(active)},
        //                                              { _self, _state.get().fund_name, value, std::string("stake tokens") } );

        print("ADDING THE RECEIPT\n");
        r_table.emplace(_self, [&](rec_entry &rec) {
            rec.id = r_table.available_primary_key();
            rec.external_id = external_id;
            rec.airdrop_id = airdrop_id;
            rec.amount = amount;
            rec.acc_name = acc_name;
            rec.symbol = symbol;
        });
    }

    EOSIO_ABI( uos_airdrop, (send))
}
