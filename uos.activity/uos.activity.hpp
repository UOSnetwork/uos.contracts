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
        void savetran(string blocknum, string transaction_id, string timestamp, string action_name, string data);

        //@abi action
        void erasetrans(uint64_t number);

    private:

        ///@abi table rate i64
        struct rate {
            uint64_t key;
            checksum256 name_hash;
            string value;
            string acc_name;

            uint64_t primary_key() const { return key; }

            key256 by_name() const { return get_name(name_hash); }

            static key256 get_name(const checksum256 &name) {
                const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&name);
                return key256::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]);
            }

            EOSLIB_SERIALIZE(rate, (key)(name_hash)(value)(acc_name))
        };

        typedef eosio::multi_index<N(rate), rate, indexed_by<N(
                name_hash), const_mem_fun<rate, key256, &rate::by_name>>> rateIndex;

        ///@abi table tran i64
        struct tran {
            uint64_t key;
            string blocknum;
            string transaction_id;
            string timestamp;
            string action_name;
            string data;

            uint64_t primary_key() const { return key; }

            key256 by_trid() const { return  get_trid_key(transaction_id); }

            static key256 get_trid_key(const string &trid) {
                checksum256 result;
                sha256((char *) trid.c_str(), strlen(&trid[0]), &result);
                const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&result);
                return key256::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]);
            }

            EOSLIB_SERIALIZE(tran, (key)(blocknum)(transaction_id)(timestamp)(action_name)(data))
        };

        typedef eosio::multi_index<N(tran), tran, indexed_by<N(
                tran_hash), const_mem_fun<tran, key256, &tran::by_trid>>> trans;
    };

    EOSIO_ABI(uos_activity, (usertouser)(makecontent)(usertocont)(setrate)(eraserate)(erase)(savetran)(erasetrans))

}