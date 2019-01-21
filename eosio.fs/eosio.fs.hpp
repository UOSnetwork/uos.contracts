/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
//#include <eosiolib/crypto.h>
//#include <eosiolib/public_key.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/asset.hpp>
//#include <eosiolib/singleton.hpp>
#include <eosiolib/core_symbol.hpp>
#include <eosio.token/eosio.token.hpp>
#include <string>
//#include <cstdint>
//#include <vector>

#define FS_ALLOC_SPACE 1000000000
#define FS_SLICE_SIZE  10000000
#define FS_START_PRICE 1000000000

namespace uos{
    using namespace eosio;
    using std::string;

    class eosio_fs: public contract {
    public:
        eosio_fs(account_name self): contract(self){
            //first init
            print("construct");
            //todo add(?) singleton for save state
            userfs_table fstab(_self,_self);
            auto itr = fstab.find(_self);
            if(itr == fstab.end()){
                fstab.emplace(_self,[&](userfs_info &item){
                    item.owner=_self;
                    item.fs_in_use = 0;
                    item.fs_all_space = 0;
                    item.fs_allocated_space = 0;
                    item.rsa_open_key="";
                });
                asset price;
                price.symbol = CORE_SYMBOL;
                price.amount = FS_START_PRICE;
                lots_table lots(_self,_self);
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

        //@abi action
        void sellfs(const account_name acc, uint64_t amount_bytes, asset price);

        //@abi action
        void buyfs(const account_name acc, uint64_t lot);//

        //@abi action
        void getbackfs(const account_name acc, uint64_t lot);

        //@abi action
        void addspace(uint64_t amount); //only self can use //todo check max size

        //@abi action
        void savekeyrsa(const account_name owner, string key);

        //@abi action
        void changealloc(const account_name owner,int64_t amount_bytes);//can use positive and negative values

        //@abi action
        void addused(const account_name fsacc, const account_name acc, uint64_t amount_bytes); //for fs_storage_accounts

        //@abi action
        void freeused(const account_name fsacc, const account_name acc, uint64_t amount_bytes); //for fs_storage_accounts


    private:
        //@abi table userfs i64
        struct userfs_info{
            account_name owner;
            uint64_t fs_all_space;          //free space = fs_all_space - fs_allocated_space. Free space can be sold.
            uint64_t fs_allocated_space;    //space, allowed to use right now
            uint64_t fs_in_use;             //fs_in_use cannot be larger than fs_allocated_space
            string rsa_open_key;

            uint64_t primary_key() const {return owner;}

            EOSLIB_SERIALIZE(userfs_info,(owner)(fs_all_space)(fs_allocated_space)(fs_in_use)(rsa_open_key))
        };

        //@abi table activelots i64
        struct lots_info{
            uint64_t lot_number;
            account_name owner;
            uint64_t fs_space;
            asset price;

            uint64_t primary_key() const {return lot_number;}

            EOSLIB_SERIALIZE(lots_info, (lot_number)(owner)(fs_space)(price))
        };

        typedef multi_index<N(userfs), userfs_info> userfs_table;

        typedef multi_index<N(activelots),lots_info> lots_table;




    };
}


