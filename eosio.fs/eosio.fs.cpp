#include <eosio.fs.hpp>

namespace uos{

    void eosio_fs::sellfs(const name acc, uint64_t amount_bytes, asset price){

        eosio_assert(price.symbol == symbol(CORE_SYMBOL,4),"not valud currency");
        require_auth(acc);
        userfs_table fstab(_self,acc.value);
        auto itr = fstab.find(acc.value);
        eosio_assert(itr != fstab.end(), "you should buy a free space firstly");

        if(amount_bytes>(itr->fs_all_space-itr->fs_allocated_space))
            amount_bytes = itr->fs_all_space-itr->fs_allocated_space;

        fstab.modify(itr,acc,[&](userfs_info &item){
                item.fs_all_space-=amount_bytes;
                eosio_assert(item.fs_all_space>=item.fs_allocated_space,"double check");
        });

        lots_table lots(_self,_self.value);
        lots.emplace(_self,[&](lots_info &litem){
                litem.owner = acc;
                litem.price = price;
                litem.fs_space = amount_bytes;
                litem.lot_number = lots.available_primary_key();
        });

    }

    void eosio_fs::buyfs(const name acc, uint64_t lot) {
        require_auth(acc);
        lots_table lots(_self,_self.value);
        auto lot_itr = lots.find(lot);
        eosio_assert(lot_itr != lots.end(), "Lot not found");
        print("transfer tokens");
        //INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {acc, N(eosio.code)},{acc,lot_itr->owner,lot_itr->price,std::string("buy fs")});
        auto price = lot_itr->price;
        price.amount = price.amount;
        action(
                permission_level{acc,"eosio.code"_n},
                "eosio.token"_n,
                "transfer"_n,
                std::make_tuple(acc,lot_itr->owner,price,std::string("buy fs"))
                ).send();

        //when tokens was transfered
        userfs_table fstab(_self,acc.value);
        auto user_itr = fstab.find(acc.value);
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
        ::require_auth2(_self.value,"active"_n.value);
        userfs_table fstab(_self,_self.value);
        auto itr = fstab.find(_self.value);
        eosio_assert(itr!=fstab.end(),"??");
        eosio_assert(amount>=FS_SLICE_SIZE,"too small amount to add");
        fstab.modify(itr,_self,[&](userfs_info &item){
            item.fs_all_space+=amount;
        });
        
        //INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {acc, N(eosio.code)},{acc,lot_itr->owner,lot_itr->price,std::string("add fs")});
        asset price;
        price.symbol = symbol(CORE_SYMBOL,4);
        price.amount = FS_START_PRICE;
        for(uint64_t i = 0; i< amount;){
            sellfs(_self, FS_SLICE_SIZE, price);
            i+=FS_SLICE_SIZE;
        }
    }

    void eosio_fs::savekeyrsa(const name owner, std::string key) {
        require_auth(owner);
        userfs_table fstab(_self,owner.value);
        auto user_itr = fstab.find(owner.value);
        eosio_assert(user_itr != fstab.end(), "you should buy a free space firstly");
        fstab.modify(user_itr,owner,[&](userfs_info &item){
            item.rsa_open_key=key;
        });
    }

    void eosio_fs::changealloc(const name owner, int64_t amount_bytes) {
        require_auth(owner);
        eosio_assert(amount_bytes!=0,"amount shouldn't be zero");
        userfs_table fstab(_self,owner.value);
        auto user_itr = fstab.find(owner.value);
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

    void eosio_fs::getbackfs(const name acc, uint64_t lot) {
        require_auth(acc);
        lots_table fslots(_self,_self.value);
        auto lot_itr = fslots.find(lot);
        eosio_assert(lot_itr!=fslots.end(),"lot not found");
        eosio_assert(lot_itr->owner==acc,"you must be owner of this lot");

        userfs_table fstab(_self,acc.value);
        auto user_itr = fstab.find(acc.value);
        eosio_assert(user_itr!=fstab.end(),"you should buy a free space firstly");

        fstab.modify(user_itr,acc,[&](userfs_info &item){
            item.fs_all_space+=lot_itr->fs_space;
        });
        fslots.erase(lot_itr);
    }

    void eosio_fs::addused(const name fsacc, const name acc, uint64_t amount_bytes) {
        require_auth(fsacc);
        userfs_table fstab(_self,acc.value);
        auto user_itr = fstab.find(acc.value);
        eosio_assert(user_itr!=fstab.end(),"user information not found");
        eosio_assert((user_itr->fs_allocated_space-user_itr->fs_in_use)>=amount_bytes,"not enough allocated space");
        if(fsacc!=_self){
            INLINE_ACTION_SENDER(eosio_fs,addused)(_self,{_self,"active"_n},{_self,acc,amount_bytes});
        }
        else{
            fstab.modify(user_itr,acc,[&](userfs_info &item){
                item.fs_in_use+=amount_bytes;
            });
        }
    }
    void eosio_fs::freeused(const name fsacc, const name acc, uint64_t amount_bytes) {
        require_auth(fsacc);
        userfs_table fstab(_self,acc.value);
        auto user_itr = fstab.find(acc.value);
        eosio_assert(user_itr!=fstab.end(),"user information not found");
        //eosio_assert(user_itr->fs_in_use>amount_bytes,"something wrong");
        if(user_itr->fs_in_use>amount_bytes)
            amount_bytes=user_itr->fs_in_use;
        if(fsacc!=_self){
            INLINE_ACTION_SENDER(eosio_fs,freeused)(_self,{_self,"active"_n},{_self,acc,amount_bytes});
        }
        else{
            fstab.modify(user_itr,acc,[&](userfs_info &item){
                item.fs_in_use-=amount_bytes;
            });
        }
    }



    EOSIO_DISPATCH(eosio_fs,(sellfs)(buyfs)(getbackfs)(addspace)(savekeyrsa)(changealloc)(addused)(freeused))

}

