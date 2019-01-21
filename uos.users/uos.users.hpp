#pragma once


#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/stdlib.hpp>
#include <eosiolib/chain.h>

#include <string>
#include <sstream>

namespace UOS {
    using namespace eosio;
    using std::string;
    using std::endl;

    class uos_users : public contract {
    public:
        uos_users(account_name self) : contract(self) {}


        //@abi action
        void delusr(account_name acc);

        //@abi action
        void lstparam(account_name acc);

        //@abi action
        void delparam(account_name acc, string param_name);


        //@abi action
        void edtparam(account_name acc, string param_name, string param_value);


    private:

//@abi type
        struct param_value {
            string param_name;
            string value;
        };

//@abi table users i64
        struct user_plist {
            account_name account;
            vector <param_value> plist; //list of indixes of parameters for this account
            uint64_t plist_size;

            uint64_t primary_key() const { return account; } //indexed by account
            void inc_size() { plist_size++; }

            void dec_size() { plist_size--; }
        };

        typedef eosio::multi_index<N(users), user_plist> users_table;


    };
}



