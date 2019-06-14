#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/stdlib.hpp>
#include <vector>


using namespace eosio;
class [[eosio::contract("hello.merkle")]] hello_merkle : public eosio::contract {
public:
    using contract::contract;

    typedef std::vector<checksum256> ch_pair;

    hello_merkle(name receiver, name code, datastream<const char*> ds)
            : contract(receiver, code, ds){}

    [[eosio::action]]
    void hi( name user ) {
        print( "Hello, ", user ,"\n");
        char hello[10]={'h','e','l','l','o',0,0,0,0,0};
        checksum256 temp;
        temp = sha256(hello, sizeof(hello));
        printhex( &temp, sizeof(temp) );
    }

    [[eosio::action]]
    void hi2(std::vector<ch_pair> input){
        for(auto item: input){
            printhex( &item[0], sizeof(item[0]));
            print(";");
            printhex( &item[1], sizeof(item[1]));
            print("\n");
            print((char*)inp);
            print("\n");
            checksum256 inp[2] = {item[0],item[1]};
            checksum256 temp;
            temp = sha256((char *)inp, sizeof(checksum256[2]));
            printhex( &temp, sizeof(temp) );
            print("\n");
        }
    }
};