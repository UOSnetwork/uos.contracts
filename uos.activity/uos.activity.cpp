#include "uos.activity.hpp"

namespace UOS {
    void uos_activity::usertouser(const account_name acc_from, const account_name acc_to, uint8_t interaction_type_id)
    {
        print("usertouser acc_from = ", name{acc_from}, " acc_to = ", name{acc_to}, " interaction_type_id = ", (int)interaction_type_id);
        eosio_assert( acc_from != acc_to, "cannot transfer to self" );
        require_auth( acc_from );
        eosio_assert( is_account( acc_to ), "to account does not exist");

    }

    void uos_activity::makecontent(account_name acc, string content_id, uint8_t content_type_id, string parent_content_id)
    {
        print("makecontent acc = ", name{acc}, " content_id = ", content_id, " content_type_id = ", (int)content_type_id, " parent_content_id = ", parent_content_id);
        require_auth( acc );

    }


    void uos_activity::usertocont(account_name acc, string content_id, uint8_t interaction_type_id)
    {
        print("usertocont acc = ", name{acc}, " content_id = ", content_id, " interaction_type_id = ", (int)interaction_type_id);
        require_auth( acc);
    }

    void uos_activity::setrate(string name, string value) {
        require_auth(_self);
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

    void uos_activity::savetran(string blocknum, string transaction_id, string timestamp, string action_name, string data) {
        require_auth(_self);

        trans tr_index(_self, _self);

        auto secondary_index = tr_index.get_index<N(tran_hash)>();
        auto itr = secondary_index.lower_bound(tran::get_trid_key(transaction_id));

        if (itr->transaction_id == transaction_id) {
            //do nothing, transaction is already saved
        } else {
            //save transaction info

            tr_index.emplace(_self, [&](auto &t) {
                t.key = tr_index.available_primary_key();;
                t.blocknum = blocknum;
                t.transaction_id = transaction_id;
                t.timestamp = timestamp;
                t.action_name = action_name;
                t.data = data;
            });
        }
    }

    void uos_activity::erasetrans(uint64_t number = 0) {
        require_auth(_self);
        trans tr_index(_self, _self);
        if (number == 0) {
            for (; tr_index.begin() != tr_index.end();)
                tr_index.erase(tr_index.begin());
        } else {
            for (uint64_t i = 0; i < number; i++) {
                if (tr_index.begin() != tr_index.end()) {
                    tr_index.erase(tr_index.begin());
                } else {
                    //print("empty");
                    break;
                }
            }

        }
    }
}