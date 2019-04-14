#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/stdlib.hpp>
using namespace eosio;

class hello_merkle : public eosio::contract {
  public:
      using contract::contract;

      typedef vector<checksum256> ch_pair;

      /// @abi action 
      void hi( account_name user ) {
         print( "Hello, ", name{user} ,"\n");
         char hello[10]={'h','e','l','l','o',0,0,0,0,0};
         checksum256 temp;
         sha256(hello, sizeof(hello),&temp);
          printhex( &temp, sizeof(temp) );
      }

      /// @abi action
      void hi2(vector<ch_pair> input){
          for(auto item: input){
              checksum256 inp[2] = {item[0],item[1]};
              checksum256 temp;
              sha256((char *)inp, sizeof(checksum256[2]),&temp);
              printhex( &temp, sizeof(temp) );
              print("\n");
          }
      }
};

EOSIO_ABI( hello_merkle, (hi)(hi2) )
