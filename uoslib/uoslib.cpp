#include "uostest.hpp"
#include <eosiolib/contract.hpp>
#include <eosiolib/transaction.hpp>
#include <set>


namespace uos{

    void hello(){};

    std::set<eosio::permission_level,permissions_comparator> get_permissions(){
        auto s = transaction_size();
        char buf[s];
        auto size = read_transaction(buf, s);
        eosio::transaction trx = eosio::unpack<eosio::transaction>(buf, s);
        trx.
        std::set<eosio::permission_level,permissions_comparator> ret;
        for(auto i: trx.actions){
            for( auto acc : i.authorization)
                ret.insert(acc);
        }
        return ret;
    }

}
