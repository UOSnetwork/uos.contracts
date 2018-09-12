
#pragma once


#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/public_key.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/core_symbol.hpp>
#include <string>
#include <cstdint>
#include <vector>

namespace UOS{
    using namespace eosio;
    using std::string;

    class uos_calculator: public contract {
    public:
        uos_calculator(account_name self) : contract(self),_state(_self,_self){
                if(!_state.exists()){
                        contract_state temp;
                        temp.base_asset = asset();
                        temp.fund_name = N(uos.cstake);
                        _state.set(temp,_self);
                }
        }

        //@abi action
        void regcalc(const account_name acc, const eosio::public_key& key, const string& url, uint16_t location);
        //void regcalc(const account_name acc, );

        //@abi action
        void rmcalc(const account_name acc);

        //@abi action
        void unregcalc(const account_name acc);

        //@abi action
        void iscalc(const account_name acc); //todo: check if account is allowed to calculate activity index. It will use assert. May be add statistics for calculators? +-

        //@abi action
        void stake(const account_name acc, const asset value); //todo: transfer money to stake account +

        //@abi action
        void refund(const account_name acc); //todo: transfer money back from stake account +

        //@abi action
        void votecalc(const account_name acc, std::vector<account_name> calcs);//todo: vote for calcs

        //@abi action
        void setasset(const asset value); //todo

        bool check_calc(const account_name calc);

    private:

        //calc_info is modified producer_info


        //@abi table state i64
        struct contract_state{
            asset base_asset;
            account_name fund_name;
            //todo
        };

        //@abi table calcs i64
        struct calc_info{
            account_name          owner;
            double                total_votes = 0;
            eosio::public_key     calc_key; /// a packed public key object
            bool                  is_active = true;
            string                url;
            uint32_t              unpaid_blocks = 0;
            uint64_t              last_claim_time = 0;
            uint16_t              location = 0;
            uint64_t primary_key() const { return owner;}
            void     deactivate()       { calc_key = eosio::public_key(); is_active = false; } //??

            EOSLIB_SERIALIZE( calc_info, (owner)(total_votes)(calc_key)(is_active)(url)(unpaid_blocks)(last_claim_time)(location) )

        };

        //@abi table voters i64
        struct voter_info{
            account_name owner=0;
            std::vector<account_name> calcs;
            asset stake;
            uint64_t primary_key() const {return owner;}

            EOSLIB_SERIALIZE (voter_info, (owner)(calcs)(stake))
        };

        typedef multi_index <N(calcs), calc_info> calcs_table;

        typedef multi_index <N(voters), voter_info> voters_table;

        typedef singleton <N(state), contract_state> contract_state_singleton;


        contract_state_singleton  _state;

    };

    EOSIO_ABI(uos_calculator,(regcalc)(rmcalc)(unregcalc)(iscalc)(stake)(refund)(votecalc)(setasset))

}
