/**
 *@brief contracts for activity
 *@version
 *  \
 **/

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/crypto.h>
#include <string.h>
#include <vector>


namespace UOS {
    using namespace eosio;
    using std::string;
    using key256=fixed_bytes<32>;

    class [[eosio::contract("uos.activity")]]uos_activity: public contract {
    public:

        uos_activity(name receiver, name code, datastream<const char*> ds)
                : contract(receiver, code, ds){}

        [[eosio::action]]
        void usertouser(const name acc_from, const name acc_to, uint8_t interaction_type_id);

        [[eosio::action]]
        void makecontent(const name acc, string content_id, uint8_t content_type_id, string parent_content_id);

        [[eosio::action]]
        void usertocont(const name acc, string content_id, uint8_t interaction_type_id);

        [[eosio::action]]
        void dirpost(const name acc, string content_id, const name acc_to, uint8_t content_type_id);

        [[eosio::action]]
        void dirpostorg(const name acc, string content_id, string organisation_to_id, uint8_t content_type_id);

        [[eosio::action]]
        void setrate(string _name, string value);

        [[eosio::action]]
        void eraserate(uint64_t index);

        /**
         * @brief erase n-record, 0 -all record
         * @param number
         */
        [[eosio::action]]
        void erase(uint64_t number);

        [[eosio::action]]
        void makecontorg(const name acc, string organization_id, string content_id, uint8_t content_type_id, string parent_content_id);

        [[eosio::action]]
        void socialaction(const name acc, string action_json);

        [[eosio::action]]
        void socialactndt(const name acc, string action_json, string action_data);

    private:

        struct  [[eosio::table]]
        rate {
            uint64_t key;
            checksum256 name_hash;
            string value;
            string acc_name;

            uint64_t primary_key() const { return key; }

            key256 by_name() const { return get_hash(name_hash); }

            static key256 get_hash(const checksum256 &name) {
                const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&name);
                return key256::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]);
            }

            EOSLIB_SERIALIZE(rate, (key)(name_hash)(value)(acc_name))
        };

        typedef eosio::multi_index<"rate"_n, rate, indexed_by<
                "name.hash"_n, const_mem_fun<rate, key256, &rate::by_name>>> rateIndex;

    };

}
