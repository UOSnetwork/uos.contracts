/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
//#include <eosiolib/crypto.h>
//#include <eosiolib/public_key.hpp>
#include <eosiolib/name.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/symbol.hpp>
#include <eosio.token/eosio.token.hpp>
#include <string>
//#include <cstdint>
//#include <vector>

#define FS_ALLOC_SPACE 1000000000
#define FS_SLICE_SIZE  10000000
#define FS_START_PRICE 1000000000
#define CORE_SYMBOL "UOS"

namespace uos{
    using namespace eosio;
    using eosio::symbol;
    using eosio::symbol_code;
    using std::string;

    class [[eosio::contract("eosio.fs")]] eosio_fs: public contract {
    public:
        using contract::contract;
        eosio_fs(name receiver, name code, datastream<const char*> ds)
                : contract(receiver, code, ds){
            //first init
            print("construct");
            //todo add(?) singleton for save state
            userfs_table fstab(_self,_self.value);
            auto itr = fstab.find(_self.value);
            if(itr == fstab.end()){
                fstab.emplace(_self,[&](userfs_info &item){
                    item.owner=_self;
                    item.fs_in_use = 0;
                    item.fs_all_space = 0;
                    item.fs_allocated_space = 0;
                    item.rsa_open_key="";
                });
                asset price;
                price.symbol = symbol(CORE_SYMBOL,4);
                price.amount = FS_START_PRICE;
                lots_table lots(_self,_self.value);
                for(auto i = 0; i<FS_ALLOC_SPACE; ){
                    lots.emplace(_self,[&](lots_info &litem){
                        litem.owner = _self;
                        litem.price = price;
                        litem.fs_space = FS_SLICE_SIZE;
                        litem.lot_number = lots.available_primary_key();
                    });
                    i= i + FS_SLICE_SIZE;
                }
            }
        }

        [[eosio::action]]
        void sellfs(const name acc, uint64_t amount_bytes, asset price);

        [[eosio::action]]
        void buyfs(const name acc, uint64_t lot);//

        [[eosio::action]]
        void getbackfs(const name acc, uint64_t lot);

        [[eosio::action]]
        void addspace(uint64_t amount); //only self can use //todo check max size

        [[eosio::action]]
        void savekeyrsa(const name owner, string key);

        [[eosio::action]]
        void changealloc(const name owner,int64_t amount_bytes);//can use positive and negative values

        [[eosio::action]]
        void addused(const name fsacc, const name acc, uint64_t amount_bytes); //for fs_storage_accounts

        [[eosio::action]]
        void freeused(const name fsacc, const name acc, uint64_t amount_bytes); //for fs_storage_accounts


    private:
        struct [[eosio::table]] userfs_info{
            name owner;
            uint64_t fs_all_space;          //free space = fs_all_space - fs_allocated_space. Free space can be sold.
            uint64_t fs_allocated_space;    //space, allowed to use right now
            uint64_t fs_in_use;             //fs_in_use cannot be larger than fs_allocated_space
            string rsa_open_key;

            uint64_t primary_key() const {return owner.value;}
        };

        struct [[eosio::table]] lots_info{
            uint64_t lot_number;
            name owner;
            uint64_t fs_space;
            asset price;

            uint64_t primary_key() const {return lot_number;}
        };

        typedef multi_index<"userfs"_n, userfs_info> userfs_table;

        typedef multi_index<"activelots"_n,lots_info> lots_table;

    public:
        using freeused_action    = eosio::action_wrapper<"freeused"_n,    &eosio_fs::freeused>;
        using sellfs_action      = eosio::action_wrapper<"sellfs"_n,      &eosio_fs::sellfs>;
        using buyfs_action       = eosio::action_wrapper<"buyfs"_n,       &eosio_fs::buyfs>;
        using getbackfs_action   = eosio::action_wrapper<"getbackfs"_n,   &eosio_fs::getbackfs>;
        using addspace_action    = eosio::action_wrapper<"addspace"_n,    &eosio_fs::addspace>;
        using savekeyrsa_action  = eosio::action_wrapper<"savekeyrsa"_n,  &eosio_fs::savekeyrsa>;
        using changealloc_action = eosio::action_wrapper<"changealloc"_n, &eosio_fs::changealloc>;
        using addused_action     = eosio::action_wrapper<"addused"_n,     &eosio_fs::addused>;

    };
}


