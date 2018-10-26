
#include "uos.accounter.hpp"
#include <eosio.token/eosio.token.hpp>
#include <uos_contracts/uos.activity/uos.activity.hpp>

namespace UOS{

    void uos_accounter::withdrawal(account_name owner) {
        require_auth(owner);
        print("hello");
        accounts_table actable(_self,owner);
        auto act_itr = actable.find(owner);
        eosio_assert(act_itr!=actable.end(),"Reward not found");
        double reward = act_itr->account_sum;
        print(reward);
        asset val;
        val.symbol = S(4,SYS);// todo change to CORE_SYMBOL
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
        INLINE_ACTION_SENDER(eosio::token, issue) ( N(eosio.token), {N(eosio),N(active)},{_self, val, std::string("issue tokens for account")} );
        INLINE_ACTION_SENDER(eosio::token, transfer) ( N(eosio.token), {_self, N(active)} ,{ _self, owner, val, std::string("transfer issued tokens for account")} );
    }

    void uos_accounter::withdraw(account_name owner, uint64_t sum) {
        require_auth(owner);
        double fsum = double(sum);
        accounts_table actable(_self,owner);
        auto act_itr = actable.find(owner);
        eosio_assert(act_itr!=actable.end(),"Reward not found");
        if (fsum > act_itr->account_sum ){
            fsum = act_itr->account_sum;
        }
        double reward = fsum;
        asset val;
        val.symbol = S(4,SYS);// todo change to CORE_SYMBOL
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
        INLINE_ACTION_SENDER(eosio::token, issue) ( N(eosio.token), {N(eosio),N(active)},{_self, val, std::string("issue tokens for account")} );
        INLINE_ACTION_SENDER(eosio::token, transfer) ( N(eosio.token), {_self, N(active)} ,{ _self, owner, val, std::string("transfer issued tokens for account")} );
    }

    void uos_accounter::addsum(account_name issuer, account_name receiver, double sum, std::string message) {
        require_auth(issuer);
        eosio_assert(is_account(receiver),"error account name");
        eosio_assert(is_issuer(issuer),"account is not registered as issuer");
        accounts_table actable(_self,receiver);
        auto act_itr = actable.find(receiver);
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

    void uos_accounter::regissuer(account_name issuer) {
        require_auth(issuer);
        eosio_assert(!is_issuer(issuer),"Already registered as issuer");
        issuers_table isstable(_self,_self);
        isstable.emplace(issuer,[&](issuer_info &item){
            item.issuer=issuer;
        });
    }

    bool uos_accounter::is_issuer(account_name acc) {
        issuers_table isstable(_self,_self);
        auto iss_itr = isstable.find(acc);
        return iss_itr!=isstable.end();
    }

    EOSIO_ABI(uos_accounter,(addsum)(regissuer)(withdrawal)(withdraw))
}

