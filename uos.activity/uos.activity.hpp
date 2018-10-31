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

    class uos_activity : public contract {
    public:
        uos_activity(account_name self) : contract(self) {}

        //@abi action
        void usertouser(const account_name acc_from, const account_name acc_to, uint8_t interaction_type_id);

        //@abi action
        void makecontent(const account_name acc, string content_id, uint8_t content_type_id, string parent_content_id);

        //@abi action
        void usertocont(const account_name acc, string content_id, uint8_t interaction_type_id);

        //@abi action
        void dirpost(const account_name acc, string content_id, const account_name acc_to, uint8_t content_type_id);

        //@abi action
        void dirpostorg(const account_name acc, string content_id, string organisation_to_id, uint8_t content_type_id);

        //@abi action
        void setrate(string name, string value);

        //@abi action
        void eraserate(uint64_t index);

        /**
         * @brief erase n-record, 0 -all record
         * @param number
         */
        //@abi action
        void erase(uint64_t number);

        //@abi action
        void makecontorg(const account_name acc,string organization_id, string content_id, uint8_t content_type_id, string parent_content_id);


    private:

        ///@abi table rate i64
        struct rate {
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

        typedef eosio::multi_index<N(rate), rate, indexed_by<N(
                name_hash), const_mem_fun<rate, key256, &rate::by_name>>> rateIndex;

    };

}
