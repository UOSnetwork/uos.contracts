#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <string>
#include <vector>


using namespace eosio;
using std::string;

class uos_activity : public contract {
        using contract::contract;

    public:
        uos_activity(account_name self):contract(self) {}

        //@abi action
        void usertouser(const account_name  acc_from, const account_name  acc_to, uint8_t interaction_type_id);

        //@abi action
        void makecontent(uint64_t acc, uint128_t  content_id, uint8_t content_type_id, uint128_t parent_content_id);

        //@abi action
        void usertocont(uint64_t acc, uint128_t content_id, uint8_t interaction_type_id);

        //@abi action
        void setrate(uint128_t name, string value);

    private:

        //@abi table rate i64
        struct rate {
            uint64_t key;
            uint128_t name;
            string value;
            uint64_t primary_key() const { return key; }

            uint128_t by_name() const { return name; }

            EOSLIB_SERIALIZE(rate, (key)(name)(value))
        };

        typedef eosio::multi_index<N(rate), rate, indexed_by<N(name), const_mem_fun<rate, uint128_t, &rate::by_name>>> rateIndex;
    };

    EOSIO_ABI(uos_activity, (usertouser)(makecontent)(usertocont)(setrate))


