#include "eosio.fs.hpp"

namespace uos{

    void eosio_fs::sellfs(const account_name acc, uint64_t amount_bytes, asset price){

        eosio_assert(price.symbol == CORE_SYMBOL,"not valud currency");
        require_auth(acc);
        userfs_table fstab(_self,acc);
        auto itr = fstab.find(acc);
        eosio_assert(itr != fstab.end(), "you should buy a free space firstly");

        if(amount_bytes>(itr->fs_all_space-itr->fs_allocated_space))
            amount_bytes = itr->fs_all_space-itr->fs_allocated_space;

        fstab.modify(itr,acc,[&](userfs_info &item){
                item.fs_all_space-=amount_bytes;
                eosio_assert(item.fs_all_space>=item.fs_allocated_space,"double check");
        });

        lots_table lots(_self,_self);
        lots.emplace(_self,[&](lots_info &litem){
                litem.owner = acc;
                litem.price = price;
                litem.fs_space = amount_bytes;
                litem.lot_number = lots.available_primary_key();
        });

    }

    void eosio_fs::buyfs(const account_name acc, uint64_t lot) {
        require_auth(acc);
        lots_table lots(_self,_self);
        auto lot_itr = lots.find(lot);
        eosio_assert(lot_itr != lots.end(), "Lot not found");
        INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {acc, N(active)},{acc,lot_itr->owner,lot_itr->price,std::string("buy fs")});
        print("transfer tokens");
        //when tokens was transfered
        userfs_table fstab(_self,acc);
        auto user_itr = fstab.find(acc);
        if(user_itr == fstab.end()){
            fstab.emplace(acc,[&](userfs_info &item){
                item.owner = acc;
                item.rsa_open_key="";
                item.fs_allocated_space = 0;
                item.fs_in_use = 0;
                item.fs_all_space = lot_itr->fs_space;
            });
        }
        else{
            fstab.modify(user_itr,acc,[&](userfs_info &item){
                item.fs_all_space+=lot_itr->fs_space;
            });
        }
        lots.erase(lot_itr);
    }

    void eosio_fs::addspace(uint64_t amount) {
        if(amount == 0) return;
        require_auth2(_self,N(active));
        userfs_table fstab(_self,_self);
        auto itr = fstab.find(_self);
        eosio_assert(itr!=fstab.end(),"??");
        eosio_assert(amount>=FS_SLICE_SIZE,"too small amount to add");
        fstab.modify(itr,_self,[&](userfs_info &item){
            item.fs_all_space+=amount;
        });
        asset price;
        price.symbol = CORE_SYMBOL;
        price.amount = FS_START_PRICE;
        for(uint64_t i = 0; i< amount;){
            sellfs(_self, FS_SLICE_SIZE, price);
            i+=FS_SLICE_SIZE;
        }
    }

    void eosio_fs::savekeyrsa(const account_name owner, std::string key) {
        require_auth(owner);
        userfs_table fstab(_self,owner);
        auto user_itr = fstab.find(owner);
        eosio_assert(user_itr != fstab.end(), "you should buy a free space firstly");
        fstab.modify(user_itr,owner,[&](userfs_info &item){
            item.rsa_open_key=key;
        });
    }

    void eosio_fs::changealloc(const account_name owner, int64_t amount_bytes) {
        require_auth(owner);
        eosio_assert(amount_bytes!=0,"amount shouldn't be zero");
        userfs_table fstab(_self,owner);
        auto user_itr = fstab.find(owner);
        eosio_assert(user_itr != fstab.end(), "you should buy a free space firstly");
        if(amount_bytes>0) {
            auto tmp = static_cast<uint64_t >(amount_bytes);
            eosio_assert((user_itr->fs_allocated_space + tmp) <= user_itr->fs_all_space,
                         "Not enough free space. You should buy it");

            fstab.modify(user_itr, owner, [&](userfs_info &item) {
                item.fs_allocated_space += tmp;
            });
        }
        else{
            amount_bytes=0-amount_bytes;
            auto tmp = static_cast<uint64_t >(amount_bytes);
            if((user_itr->fs_allocated_space-user_itr->fs_in_use)<tmp)
                tmp = user_itr->fs_allocated_space-user_itr->fs_in_use;
            else
            if(tmp>user_itr->fs_allocated_space)
                tmp = user_itr->fs_allocated_space;
            fstab.modify(user_itr,owner,[&](userfs_info &item){
                item.fs_allocated_space-=tmp;
            });
        }
    }

    void eosio_fs::getbackfs(const account_name acc, uint64_t lot) {
        require_auth(acc);
        lots_table fslots(_self,_self);
        auto lot_itr = fslots.find(lot);
        eosio_assert(lot_itr!=fslots.end(),"lot not found");
        eosio_assert(lot_itr->owner==acc,"you must be owner of this lot");

        userfs_table fstab(_self,acc);
        auto user_itr = fstab.find(acc);
        eosio_assert(user_itr!=fstab.end(),"you should buy a free space firstly");

        fstab.modify(user_itr,acc,[&](userfs_info &item){
            item.fs_all_space+=lot_itr->fs_space;
        });
        fslots.erase(lot_itr);
    }

    void eosio_fs::addused(const account_name fsacc, const account_name acc, uint64_t amount_bytes) {
        require_auth(fsacc);
        userfs_table fstab(_self,acc);
        auto user_itr = fstab.find(acc);
        eosio_assert(user_itr!=fstab.end(),"user information not found");
        eosio_assert((user_itr->fs_allocated_space-user_itr->fs_in_use)>=amount_bytes,"not enough allocated space");
        if(fsacc!=_self){
            INLINE_ACTION_SENDER(eosio_fs,addused)(_self,{_self,N(active)},{_self,acc,amount_bytes});
        }
        else{
            fstab.modify(user_itr,acc,[&](userfs_info &item){
                item.fs_in_use+=amount_bytes;
            });
        }
    }

   void eosio_fs::getstats(const account_name accname) {
        userfs_table fstab(_self,acc);
        auto itr = fstab.find(acc);
        require_auth(accname);
        eosio_assert(price.symbol == CORE_SYMBOL,"not valud currency");
        userfs_table fstab(_self,accname);
        auto user_itr = fstab.find(accname);
        if(user_itr->fs_in_use>amount_bytes)
            amount_bytes=user_itr->fs_in_use;
        eosio_assert(user_itr!=fstab.end(),"user information not found");
        eosio_assert((user_itr->fs_allocated_space-user_itr->fs_in_use)>=amount_bytes,"not enough allocated space");
        eosio_assert(lot_itr->owner==accname,"you must be owner of this lot");
        fstab.modify(user_itr,acc,[&](userfs_info &item){
                item.fs_in_use+=amount_bytes;
        if(tmp>user_itr->fs_allocated_space)
                tmp = user_itr->fs_allocated_space;
        if(fsacc!=_self){
           INLINE_ACTION_SENDER(eosio_fs,addused)(_self,{_self,N(active)},{_self,acc,amount_bytes});
        }
        else{
            fstab.modify(user_itr,acc,[&](userfs_info &item){
                item.fs_in_use+=amount_bytes;
            });
        INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {acc, N(active)},{acc,lot_itr->owner,lot_itr->price,std::string("buy fs")});
        print("get stats");
    }
    
    void eosio_fs::freeused(const account_name fsacc, const account_name acc, uint64_t amount_bytes) {
        require_auth(fsacc);
        userfs_table fstab(_self,acc);
        auto user_itr = fstab.find(acc);
        eosio_assert(user_itr!=fstab.end(),"user information not found");
        //eosio_assert(user_itr->fs_in_use>amount_bytes,"something wrong");
        if(user_itr->fs_in_use>amount_bytes)
            amount_bytes=user_itr->fs_in_use;
        if(fsacc!=_self){
            INLINE_ACTION_SENDER(eosio_fs,freeused)(_self,{_self,N(active)},{_self,acc,amount_bytes});
        }
        else{
            fstab.modify(user_itr,acc,[&](userfs_info &item){
                item.fs_in_use-=amount_bytes;
            });
        }
    }



    EOSIO_ABI(eosio_fs,(sellfs)(buyfs)(getbackfs)(addspace)(savekeyrsa)(changealloc)(addused)(freeused)(getstats))

}

