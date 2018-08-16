#include "uos.activity.hpp"


    void uos_activity::usertouser(const account_name acc_from, const account_name acc_to,uint8_t interaction_type_id)
    {

    }

    void uos_activity::makecontent(uint64_t acc, uint128_t content_id, uint8_t content_type_id, uint128_t parent_content_id)
    {

    }


    void uos_activity::usertocont(uint64_t acc, uint128_t content_id, uint8_t interaction_type_id)
    {

    }

    void uos_activity::setrate(uint128_t name, string value)
    {

        //       require_auth(_self);
        rateIndex rates(_self, _self);
        auto iterator = rates.find(_self);
        eosio_assert(iterator == rates.end(), "Address for account already exists");

        rates.emplace(_self, [&](auto &rate) {
            rate.key = rates.available_primary_key();
            rate.param = name;
            rate.value = value;
        });
    }
