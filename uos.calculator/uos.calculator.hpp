
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
                        temp.fund_name = N(uos.stake);
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
        void iscalc(const account_name acc);                    //check if account is allowed to calculate activity index. It will use assert. May be add statistics for calculators?

        //@abi action
        void stake(const account_name voter, const asset value);  //transfer money to stake account

        //@abi action
        void refund(const account_name voter);                    //todo: transfer money back from stake account +

        //@abi action
        void votecalc(const account_name voter, std::vector<account_name> calcs);

        //@abi action
        void unvote(const account_name voter, std::vector<account_name> calcs);

        //@abi action
        void unvoteall(const account_name voter);

        //@abi action
        void setasset(const asset value);                       //todo +-

        bool check_calc(const account_name calc);

        asset get_stake(account_name);


////    from uos.accounter {

        //@abi action
        void withdrawal(account_name owner);

        //@abi action
        void withdraw(account_name owner, uint64_t sum);

        ////sender operations
        //@abi action
        void addsum(account_name issuer, account_name receiver, double sum, string message);

        //@abi action
        void regissuer(account_name issuer);

////    } from uos.accounter

////    from uos.activity {

        //@abi action
        void setrate(string name, string value);

        //@abi action
        void eraserate(uint64_t index);

        /**
         * @brief erase n-record, 0 -all record
         * @param number
         */
        //@abi action
        void erase(uint64_t number);

        //@abi action
        void makecontorg(const account_name acc,string organization_id, string content_id, uint8_t content_type_id, string parent_content_id);

////    } from uos.activity

    private:

        //  const CALC_NUM = 8;

        //  calc_info is modified producer_info


        //@abi table state i64
        struct contract_state{
            asset base_asset;
            account_name fund_name;
            //todo
        };

        //@abi table calcs i64
        struct calc_info{
            account_name          owner;
            uint64_t              total_votes = 0;
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

        struct candidate_info{
            account_name calc;
            int64_t bid;
        };

        //@abi table voters i64
        struct voter_info{
            account_name owner=0;
            std::vector<candidate_info> calcs;
            asset stake;
            asset stake_voted;
            uint64_t primary_key() const {return owner;}

            EOSLIB_SERIALIZE (voter_info, (owner)(calcs)(stake)(stake_voted))
        };




        typedef multi_index <N(calcs), calc_info> calcs_table;

        typedef multi_index <N(voters), voter_info> voters_table;

        typedef singleton <N(state), contract_state> contract_state_singleton;


        contract_state_singleton  _state;

        bool unvote_vector(voters_table &voters, voters_table::const_iterator &itr, const account_name &voter, std::vector<account_name> &calcs_to_unvote);

////         from uos.accounter {

        //@abi table account i64
        struct account_info{
            account_name owner;
            double account_sum;

            uint64_t  primary_key() const {return owner;}

            EOSLIB_SERIALIZE(account_info, (owner)(account_sum))
        };

        //@abi table issuers i64
        struct issuer_info{
            account_name issuer;

            uint64_t primary_key() const {return issuer;}

            EOSLIB_SERIALIZE(issuer_info,(issuer))
        };


        typedef multi_index <N(account),account_info> accounts_table;
        typedef multi_index <N(issuers), issuer_info> issuers_table;

        bool is_issuer(account_name acc);

////         } from uos.accounter

////         from uos.activity {


        ///@abi table rate i64
        struct rate {
            uint64_t key;
            checksum256 name_hash;
            string value;
            string acc_name;

            uint64_t primary_key() const { return key; }

            key256 by_name() const { return get_hash(name_hash); }

            static key256 get_hash(const checksum256 &name) {
                const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&name);
                return key256::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]);
            }

            EOSLIB_SERIALIZE(rate, (key)(name_hash)(value)(acc_name))
        };

        typedef eosio::multi_index<N(rate), rate, indexed_by<N(
                name_hash), const_mem_fun<rate, key256, &rate::by_name>>> rateIndex;

////        } from uos.activity

    };

}
