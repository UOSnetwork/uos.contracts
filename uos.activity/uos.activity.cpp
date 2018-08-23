#include "uos.activity.hpp"

namespace UOS {
    void uos_activity::usertouser(const account_name acc_from, const account_name acc_to, uint8_t interaction_type_id) {

    }

    void
    uos_activity::makecontent(account_name acc, string content_id, uint8_t content_type_id, string parent_content_id) {

    }


    void uos_activity::usertocont(account_name acc, string content_id, uint8_t interaction_type_id) {

    }

    void uos_activity::setrate(string name, string value) {
        //require_auth(_self);
        checksum256 result;
        sha256((char *) name.c_str(), strlen(&name[0]), &result);
        rateIndex rates(_self, _self);

        string name_acc = name;
        const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&result);
        auto secondary_index = rates.get_index<N(name_hash)>();
        auto itr = secondary_index.lower_bound(
                key256::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]));
        auto end_itr = secondary_index.end();
        auto beg_itr = secondary_index.begin();

        if (itr->acc_name == name_acc) {
//        secondary_index.erase(itr);//erase should be failed
            auto iter_rate = rates.find(itr->key);
            eosio_assert(iter_rate != rates.end(), "Rate is key not found");

            rates.modify(iter_rate, _self, [&](rate &item) {
                item.value = value;
            });
        } else {
            auto iterator = rates.find(_self);
            eosio_assert(iterator == rates.end(), "Primary key already exists");

            rates.emplace(_self, [&](auto &rate) {
                rate.key = rates.available_primary_key();
                rate.name_hash = result;
                rate.value = value;
                rate.acc_name = name_acc;
            });
        }

//    for( const auto& item :  secondary_index ) {
//        print(" ID=", item.primary_key(), ", value=", item.value, "\n");
//    }

    }

    void uos_activity::eraserate(uint64_t index) {
        require_auth(_self);
        rateIndex rates(_self, _self);
        auto iter = rates.find(index);
        rates.erase(rates.get(index));
    }

    void uos_activity::erase(uint64_t number = 0) {
        require_auth(_self);
        rateIndex rates(_self, _self);
        if (number == 0) {
            for (; rates.begin() != rates.end();)
                rates.erase(rates.begin());
        } else {
            for (uint64_t i = 0; i < number; i++) {
                if (rates.begin() != rates.end()) {
                    rates.erase(rates.begin());
                } else {
                    //print("empty");
                    break;
                }
            }

        }
    }
}