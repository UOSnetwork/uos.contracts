
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

        typedef multi_index <N(calcs), calc_info> calcs_table;
        typedef multi_index <N(calcreg), calc_register> calcreg_table;

    };

    EOSIO_ABI(uos_calculator,(regcalc)(rmcalc)(unregcalc)(iscalc)(setallcalc))

}
