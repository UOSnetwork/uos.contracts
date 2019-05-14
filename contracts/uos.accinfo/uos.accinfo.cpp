#include "uos.accinfo.hpp"

namespace UOS {

    void uos_accinfo::setprofile(name acc, string profile_json) {

        require_auth(acc);

        accprofile_table userinfo(_self, acc.value);
        auto itr = userinfo.find(acc.value);
        if (itr != userinfo.end()) {
            userinfo.modify(itr, acc, [&](acc_profile &item) {
                item.profile_json = profile_json;
            });
        } else {
            userinfo.emplace(acc, [&](acc_profile &item) {
                item.profile_json = profile_json;
                item.acc = acc;

            });
        }

    }

    EOSIO_DISPATCH( uos_accinfo, (setprofile))
}
