
#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/crypto.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

#include <string>
#include <cstdint>
#include <vector>

#define CORE_SYMBOL "UOS"



namespace UOS{
    using namespace eosio;
    using std::string;
    using key256=fixed_bytes<32>;

    class [[eosio::contract("uos.calculator")]]uos_calculator: public contract {
    public:
        uos_calculator(name receiver, name code, datastream<const char*> ds)
                : contract(receiver, code, ds) , _state(code,receiver.value){
                if(!_state.exists()){
                        contract_state temp;
                        temp.base_asset = asset();
                        temp.fund_name = "uos.stake"_n;
                        _state.set(temp,_self);
                }
        }

////    from uos.accounter {

        [[eosio::action]]
        void withdrawal(name owner);

        [[eosio::action]]
        void withdraw(name owner, double sum);

        ////sender operations
        [[eosio::action]]
        void addsum(name issuer, name receiver, double sum, string message);

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

        struct  [[eosio::table("global")]]
        contract_state{
            asset base_asset;
            name fund_name;
            //todo
        };

        typedef singleton <"state"_n, contract_state> contract_state_singleton;


        contract_state_singleton  _state;

////         from uos.accounter {

        struct  [[eosio::table]]
        account_info{
            name owner;
            double account_sum;

            uint64_t  primary_key() const {return owner.value;}

            EOSLIB_SERIALIZE(account_info, (owner)(account_sum))
        };

        typedef multi_index <"account"_n,account_info> accounts_table;

////         } from uos.accounter

////         from uos.activity {

        struct  [[eosio::table]]
        rate {
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


        struct  [[eosio::table]]
        ratetr {
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

        typedef multi_index < "ratetr"_n, ratetr, indexed_by < "name.hash"_n, const_mem_fun < ratetr, key256, &ratetr::by_name > > > ratetrIndex;

////        } from uos.activity


////    from branch "direct set" {

        struct  [[eosio::table]]
        calc_register{
            name owner;

            uint64_t primary_key() const { return owner.value; }

            EOSLIB_SERIALIZE( calc_register, (owner))
        };

        typedef multi_index <"calcreg"_n, calc_register> calcreg_table;


        struct  [[eosio::table]]
        calc_reports{
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

        struct  [[eosio::table]]
        cons_block{
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
