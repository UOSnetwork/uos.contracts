#include "uos.member.hpp"

namespace UOS {
    void Members::add(account_name account, string& username) {

        require_auth(account);
        memberIndex members(_self, _self);

        auto iterator = members.find(account);
        eosio_assert(iterator == members.end(), "Address for account already exists");

        members.emplace(account, [&](auto &member) {
            member.account_name = account;
            member.username = username;
            member.level = 1;
        });
    }

    void Members::update(account_name account, uint64_t level) {
        require_auth(account);

        memberIndex members(_self, _self);

        auto iterator = members.find(account);
        eosio_assert(iterator != members.end(), "Address for account not found");


        members.modify(iterator, account, [&](auto &member) {
            member.level = level;

        });
    }
        
    void Members::getmember(const account_name account) {
        memberIndex members(_self, _self);

        auto iterator = members.find(account);
        eosio_assert(iterator != members.end(), "Address for account not found");

        auto currentMember = members.get(account);
        print("Username: ", currentMember.username.c_str());
        print(" Level: ", currentMember.level);

        if (currentMember.abilities.size() > 0) {
            print(" Abilities: ");

            for (uint32_t i = 0; i < currentMember.abilities.size(); i++) {
                print(currentMember.abilities.at(i).c_str(), " ");
            }
        } else {
            print(" No Abilities");
        }

        if (currentMember.inventory.size() > 0) {
            print(" Items: ");

            for (uint32_t i = 0; i < currentMember.inventory.size(); i++) {
                item currentItem = currentMember.inventory.at(i);
                print(currentItem.name.c_str(), " == ");
            }
        } else {
            print(" Empty inventory");
        }
    }

    void Members::addability(const account_name account, string& ability) {
        require_auth(account);

        memberIndex members(_self, _self);

        auto iterator = members.find(account);
        eosio_assert(iterator != members.end(), "Address for account not found");

        members.modify(iterator, account, [&](auto& member) {
            member.abilities.push_back(ability);
        });
    }

    void Members::additem(const account_name account, item purchased_item) {
        memberIndex members(_self, _self);

        auto iterator = members.find(account);
        eosio_assert(iterator != members.end(), "Address for account not found");

        members.modify(iterator, account, [&](auto& member) {
            member.level += purchased_item.level_up;
            member.abilities.push_back(purchased_item.ability);
            member.inventory.push_back(item{
                purchased_item.item_id,
                purchased_item.name,
                purchased_item.ability,
                purchased_item.level_up
            });
        });

        print("Item Id: ", purchased_item.item_id);
        print(" | Name: ", purchased_item.name.c_str());
        print(" | Ability: ", purchased_item.ability.c_str());
        print(" | Level up: ", purchased_item.level_up);
    }
}
