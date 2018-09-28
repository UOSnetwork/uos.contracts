
#pragma once


#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/public_key.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/singleton.hpp>
#include <string>
#include <cstdint>
#include <vector>

namespace UOS{
    using namespace eosio;
    using std::string;

    class uos_calculator: public contract {
    public:
        uos_calculator(account_name self) : contract(self) {

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

        bool check_calc(const account_name calc);

        //@abi action
        void setallcalc(vector<account_name> accounts);

        //@abi action
        void reporthash(const account_name acc, string hash, uint64_t block_num, string memo);

    private:

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

        //@abi table calcreg i64
        struct calc_register{
            account_name owner;

            uint64_t primary_key() const { return owner; }

            EOSLIB_SERIALIZE( calc_register, (owner))
        };

        //@abi table reports i64
        struct calc_reports{
            uint64_t key;
            account_name acc;
            string hash;
            uint64_t block_num;
            string memo;

            uint64_t primary_key() const { return key; }
            uint64_t by_block_num() const { return block_num; }
            key256 by_acc_block() const { return get_acc_block_hash(acc, block_num); }

            static key256 get_acc_block_hash(account_name _acc, uint64_t _block_num) {
                string concat = name{_acc}.to_string() + ";" + uint64_t_to_string((unsigned long)_block_num);
                print("acc;block_num concatenation |",concat ,"|\n");
                checksum256 checksum_result;
                sha256((char *) concat.c_str(), strlen(&concat[0]), &checksum_result);
                const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&checksum_result);
                key256 key256_result =  key256::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]);
                return  key256_result;
            }

            static std::string uint64_t_to_string(uint64_t num)
            {
                char str[20];
                sprintf(str, "%d", num);
                std::string s = str;

                return s;
            }

            EOSLIB_SERIALIZE(calc_reports, (key)(acc)(hash)(block_num)(memo) )
        };

        typedef multi_index <N(calcs), calc_info> calcs_table;
        typedef multi_index <N(calcreg), calc_register> calcreg_table;
        typedef multi_index <
                N(reports), calc_reports
                ,indexed_by<N(block_num), const_mem_fun<calc_reports, uint_fast64_t , &calc_reports::by_block_num>>
                ,indexed_by<N(acc_block), const_mem_fun<calc_reports, key256 , &calc_reports::by_acc_block>>
        > reports_table;
    };

    EOSIO_ABI(uos_calculator,(regcalc)(rmcalc)(unregcalc)(iscalc)(setallcalc)(reporthash))

}
