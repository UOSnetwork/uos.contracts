#include "uos.users.hpp"

namespace UOS {
void uos_users::edtparam(account_name acc, string param_name, string param_value) {
    print("Add or edit one param\n");
    eosio::name self;
    self.value = _self;

    users_table users(_self,acc);
    auto itr = users.find(acc);
    if(itr==users.end()) {
        users.emplace(_self, [&](auto &f) {
            f.account = acc;
            f.plist_size=0;
            bool found = false;
            for (auto item: f.plist) {
                if (item.param_name == param_name) {
                    found = true;
                    item.value = param_value;
                }
            }
            if(!found){
                f.plist.push_back({param_name,param_value});
                f.plist_size++;
            }
        });
    }
    else{
        users.modify(itr,0,[&](auto& item){
            bool found = false;
            for(auto& i: item.plist){

                if(i.param_name==param_name){
                    found = true;
                    print("Found\n");
                    //i = {param_name,param_value};
                    i.value = param_value;
                }
            }
            if(!found){
                item.plist.push_back({param_name,param_value});
                item.plist_size++;
            }
        });
    }
}

void uos_users::delparam(account_name acc, string param_name) {
    print("Delete param of user\n");
    users_table users(_self,acc);
    auto itr = users.find(acc);
    if(itr != users.end()){
        users.modify(itr, 0, [&](auto& item){
            for(auto iter=item.plist.begin(); iter!=item.plist.end();++iter){
                if(iter->param_name==param_name){
                    item.plist.erase(iter,iter+1);
                    print(item.plist_size,"--");
                    item.plist_size--;
                    print(item.plist_size, "\n");
                    break;
                }
            }
        });
    }
}

void  uos_users::lstparam(account_name acc) {
    print("List params of user\n");
    users_table users(_self,acc);
    auto itr = users.find(acc);
    if(itr != users.end()){
        print(itr->account,"\n",itr->plist_size, "\n");
        for(auto iter= itr->plist.begin();iter!=itr->plist.end();++iter){
            print("Name: ", iter->param_name, " Value: ", iter->value, "\n");
        }
    }else{
        print("There is no users\n");
    }
}

void uos_users::delusr(account_name acc) {
    print("Delete user\n");
    users_table users(_self,acc);
    auto itr = users.find(acc);
    if(itr!=users.end()){
        users.erase(itr);
    }
}

EOSIO_ABI( uos_users, (edtparam)(delparam)(lstparam)(delusr))
}