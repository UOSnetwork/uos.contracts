
#pragma once


#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/core_symbol.hpp>
#include <string>

namespace UOS {
    using namespace eosio;
    using std::string;

    class uos_accounter : public contract {
    public:
        uos_accounter(account_name self) : contract(self) {}

        ////user operations
        //@abi action
        void withdrawal(account_name owner);

        //@abi action
        void withdraw(account_name owner, uint64_t sum);

        ////sender operations
        //@abi action
        void addsum(account_name issuer, account_name receiver, double sum, string message);

        //@abi action
        void regissuer(account_name issuer);


    private:
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

    };
}
