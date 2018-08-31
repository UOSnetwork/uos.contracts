#include <string>
#include <set>
#include <eosiolib/permission.hpp>


#pragma once

namespace uos{

    std::string hello_there(){return "hello there!";}

    struct permissions_comparator{
        bool operator()( const eosio::permission_level &first, const eosio::permission_level &second) const{
            return (first.actor<second.actor)&&(first.permission<second.permission);
        }
    };

    std::set<eosio::permission_level,permissions_comparator> get_permissions();

}
