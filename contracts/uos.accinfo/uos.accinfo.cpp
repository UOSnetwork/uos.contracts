#include "uos.accinfo.hpp"

namespace UOS {

    void uos_accinfo::setprofile(account_name acc, string profile_json) {

        require_auth(acc);

        accprofile_table userinfo(_self, acc);
        auto itr = userinfo.find(acc);
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

    EOSIO_ABI( uos_accinfo, (setprofile))
}
