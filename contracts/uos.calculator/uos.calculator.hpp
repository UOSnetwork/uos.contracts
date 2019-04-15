
#pragma once

//#include <eosiolib/native/eosio/intrinsics_def.hpp>
//#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/crypto.hpp>
#include <eosiolib/public_key.hpp>
#include <eosiolib/name.hpp>
//#include <eosiolib/types.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/fixed_bytes.hpp>

#include <string>
#include <cstdint>
#include <vector>

#define CORE_SYMBOL "UOS"



namespace UOS{
    using namespace eosio;
    using std::string;
    using key256=fixed_bytes<256>;

    class [[eosio::contract("uos.calculator")]]uos_calculator: public contract {
    public:
        uos_calculator(name receiver, name code, datastream<const char*> ds)
                : contract(receiver, code, ds) , _state(code,receiver.value){
                if(!_state.exists()){
                        contract_state temp;
                        temp.base_asset = asset();
                        temp.fund_name = "uos.stake"_n;
                        _state.set(temp,_self.value);
                }
        }

        [[eosio::action]]
        void regcalc(const name acc, const eosio::public_key& key, const string& url, uint16_t location);
        //void regcalc(const account_name acc, );

        [[eosio::action]]
        void rmcalc(const name acc);

        [[eosio::action]]
        void unregcalc(const name acc);

        [[eosio::action]]
        void iscalc(const name acc);                    //check if account is allowed to calculate activity index. It will use assert. May be add statistics for calculators?

        [[eosio::action]]
        void stake(const name voter, const asset value);  //transfer money to stake account

        [[eosio::action]]
        void refund(const name voter);                    //todo: transfer money back from stake account +

        [[eosio::action]]
        void votecalc(const name voter, std::vector<name> calcs);

        [[eosio::action]]
        void unvote(const name voter, std::vector<name> calcs);

        [[eosio::action]]
        void unvoteall(const name voter);

        [[eosio::action]]
        void setasset(const asset value);                       //todo +-

        bool check_calc(const name calc);

        asset get_stake(name);


////    from uos.accounter {

        [[eosio::action]]
        void withdrawal(name owner);

        [[eosio::action]]
        void withdraw(name owner, double sum);

        ////sender operations
        [[eosio::action]]
        void addsum(name issuer, name receiver, double sum, string message);

        [[eosio::action]]
        void regissuer(name issuer);

////    } from uos.accounter

////    from uos.activity {

        [[eosio::action]]
        void setrate(string name, string value);

        [[eosio::action]]
        void eraserate(uint64_t index);

        /**
         * @brief erase n-record, 0 -all record
         * @param number
         */

        [[eosio::action]]
        void erase(uint64_t number);

        [[eosio::action]]
        void setratetran(string name, string value);

        [[eosio::action]]
        void makecontorg(const name acc,string organization_id, string content_id, uint8_t content_type_id, string parent_content_id);

////    } from uos.activity

////    from branch "direct set" {

        [[eosio::action]]
        void setallcalc(std::vector<name> accounts);

        [[eosio::action]]
        void reporthash(const name acc, string hash, uint64_t block_num, string memo);

////    } from branch "direct set"
    private:

        //  const CALC_NUM = 8;

        //  calc_info is modified producer_info


        [[eosio::table]]
        struct contract_state{
            asset base_asset;
            name fund_name;
            //todo
        };

        [[eosio::table]]
        struct calc_info{
            name                  owner;
            uint64_t              total_votes = 0;
            eosio::public_key     calc_key; /// a packed public key object
            bool                  is_active = true;
            string                url;
            uint32_t              unpaid_blocks = 0;
            uint64_t              last_claim_time = 0;
            uint16_t              location = 0;
            uint64_t primary_key() const { return owner.value;}
            void     deactivate()       { calc_key = eosio::public_key(); is_active = false; } //??

//            EOSLIB_SERIALIZE( calc_info, (owner)(total_votes)(calc_key)(is_active)(url)(unpaid_blocks)(last_claim_time)(location) )

        };

        struct candidate_info{
            name calc;
            int64_t bid;
        };

        [[eosio::table]]
        struct voter_info{
            name owner;
            std::vector<candidate_info> calcs;
            asset stake;
            asset stake_voted;
            uint64_t primary_key() const {return owner.value;}

//            EOSLIB_SERIALIZE (voter_info, (owner)(calcs)(stake)(stake_voted))
        };




        typedef multi_index <"calcs"_n, calc_info> calcs_table;

        typedef multi_index <"voters"_n, voter_info> voters_table;

        typedef singleton <"state"_n, contract_state> contract_state_singleton;


        contract_state_singleton  _state;

        bool unvote_vector(voters_table &voters, voters_table::const_iterator &itr, const name &voter, std::vector<name> &calcs_to_unvote);

////         from uos.accounter {

        [[eosio::table]]
        struct account_info{
            name owner;
            double account_sum;

            uint64_t  primary_key() const {return owner.value;}

//            EOSLIB_SERIALIZE(account_info, (owner)(account_sum))
        };

        [[eosio::table]]
        struct issuer_info{
            name issuer;

            uint64_t primary_key() const {return issuer.value;}

//            EOSLIB_SERIALIZE(issuer_info,(issuer))
        };


        typedef multi_index <"account"_n,account_info> accounts_table;
        typedef multi_index <"issuers"_n, issuer_info> issuers_table;

        bool is_issuer(name acc);

////         } from uos.accounter

////         from uos.activity {


        [[eosio::table]]
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

        typedef eosio::multi_index<"rate"_n, rate,indexed_by<"name.hash"_n, const_mem_fun<rate, key256, &rate::by_name>>> rateIndex;

        [[eosio::table]]
        struct ratetr {
            uint64_t key;
            checksum256 name_hash;
            string value;
            string acc_name;

            uint64_t primary_key() const { return key; }

            checksum256 by_name() const { return get_hash(name_hash); }

            static key256 get_hash(const checksum256 &name) {
                    const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&name);
                    return key256::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]);
            }

            EOSLIB_SERIALIZE(ratetr, (key)(name_hash)(value)(acc_name))
        };

        typedef eosio::multi_index < name{"ratetr"}, ratetr, indexed_by < name{"name.hash"}, const_mem_fun < ratetr, key256, &ratetr::by_name > > > ratetrIndex;

////        } from uos.activity


////    from branch "direct set" {

        [[eosio::table]]
        struct calc_register{
            name owner;

            uint64_t primary_key() const { return owner.value; }

            EOSLIB_SERIALIZE( calc_register, (owner))
        };

        typedef multi_index <"calcreg"_n, calc_register> calcreg_table;

        [[eosio::table]]
        struct calc_reports{
            uint64_t key;
            name acc;
            string hash;
            uint64_t block_num;
            string memo;

            uint64_t primary_key() const { return key; }
            uint64_t by_block_num() const { return block_num; }
            key256 by_acc_block() const { return get_acc_block_hash(acc, block_num); }

            static key256 get_acc_block_hash(name _acc, uint64_t _block_num) {
                string concat = name{_acc}.to_string() + ";" + uint64_t_to_string((unsigned long)_block_num);
                print("acc;block_num concatenation |",concat ,"|\n");
                checksum256 checksum_result;
                checksum_result = sha256((char *) concat.c_str(), strlen(&concat[0]));
                const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&checksum_result);
                key256 key256_result =  key256::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]);
                return  key256_result;
            }

            static std::string uint64_t_to_string(uint64_t num)
            {
                char str[20];
                sprintf(str, "%llu", num);

                return std::string(str);
            }

            EOSLIB_SERIALIZE(calc_reports, (key)(acc)(hash)(block_num)(memo) )
        };

        typedef multi_index <
                "reports"_n, calc_reports
                ,indexed_by<"block.num"_n, const_mem_fun<calc_reports, uint_fast64_t , &calc_reports::by_block_num>>
                ,indexed_by<"acc.block"_n, const_mem_fun<calc_reports, key256 , &calc_reports::by_acc_block>>
        > reports_table;

////    } from branch "direct set"

        [[eosio::table]]
        struct cons_block{
            uint64_t block_num;
            string hash;
            name leader;
            uint64_t primary_key() const {return block_num;}
            uint64_t reverse_key() const {return ~block_num;}

            EOSLIB_SERIALIZE(cons_block, (block_num)(hash)(leader) )
        };


        typedef multi_index <
                "consensus"_n, cons_block
                ,indexed_by<"reverse"_n, const_mem_fun<cons_block, uint_fast64_t, &cons_block::reverse_key>>
        > consensus_bl_table;
    };

}
