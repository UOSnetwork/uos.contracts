#include "uos.activity.hpp"

namespace UOS {
    void uos_activity::usertouser(const name acc_from, const name acc_to, uint8_t interaction_type_id)
    {
        print("usertouser acc_from = ", name{acc_from}, " acc_to = ", name{acc_to}, " interaction_type_id = ", (int)interaction_type_id);
        check( acc_from != acc_to, "cannot transfer to self" );
        require_auth( acc_from );
        check( is_account( acc_to ), "to account does not exist");

    }

    void uos_activity::makecontent(name acc, string content_id, uint8_t content_type_id, string parent_content_id)
    {
        print("makecontent acc = ", name{acc}, " content_id = ", content_id, " content_type_id = ", (int)content_type_id, " parent_content_id = ", parent_content_id);
        require_auth( acc );

    }

     void uos_activity::makecontorg(const name acc, string organization_id, std::string content_id, uint8_t content_type_id, std::string parent_content_id)
    {
        print("makecontent acc = ", name{acc}, "organization_id = ", organization_id,  " content_id = ", content_id,
              " content_type_id = ", (int)content_type_id,
              " parent_content_id = ", parent_content_id);
        require_auth( acc );

    }

    void uos_activity::usertocont(name acc, string content_id, uint8_t interaction_type_id)
    {
        print("usertocont acc = ", name{acc}, " content_id = ", content_id, " interaction_type_id = ", (int)interaction_type_id);
        require_auth( acc);
    }

    void uos_activity::dirpost(const name acc, std::string content_id, const name acc_to, uint8_t content_type_id)
    {
        print("dirpost acc = ", name{acc}, " content_id = ", content_id, " acc_to = ", name{acc_to}, " content_type_id = ", (int)content_type_id);
        require_auth(acc);
    }

    void uos_activity::dirpostorg(const name acc, std::string content_id, string organization_to_id, uint8_t content_type_id)
    {
        print("dirpostorg acc = ", name{acc}, " content_id = ", content_id, " organization_to_id = ", organization_to_id, " content_type_id = ", (int)content_type_id);
        require_auth(acc);
    }

    void uos_activity::setrate(string _name, string value) {
        require_auth(_self);
        checksum256 result;
        result = sha256((char *) _name.c_str(), strlen(&_name[0]));
        rateIndex rates(_self, _self.value);

        string name_acc = _name;
        auto secondary_index = rates.get_index<"name.hash"_n>();
        auto itr = secondary_index.lower_bound(rate::get_hash(result));

        if (itr->acc_name == name_acc) {
//        secondary_index.erase(itr);//erase should be failed
            auto iter_rate = rates.find(itr->key);
            check(iter_rate != rates.end(), "Rate is key not found");

            rates.modify(iter_rate, _self, [&](rate &item) {
                item.value = value;
            });
        } else {
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
        rateIndex rates(_self, _self.value);
        auto iter = rates.find(index);
        if(iter != rates.end())
            rates.erase(iter);
    }

    void uos_activity::erase(uint64_t number = 0) {
        require_auth(_self);
        rateIndex rates(_self, _self.value);
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

   void uos_activity::socialaction(const name acc, string action_json)
   {
       require_auth( acc );
   }

   void uos_activity::socialactndt(const name acc, string action_json, string action_data)
   {
       require_auth( acc );
   }

   void uos_activity::histactndt(const name acc, string action_json, string action_data, string timestamp)
   {
       require_auth( acc );
   }


    EOSIO_DISPATCH(uos_activity, (usertouser)(makecontent)(usertocont)(setrate)(eraserate)(erase)(makecontorg)(dirpost)(dirpostorg)(socialaction)(socialactndt)(histactndt))

}
