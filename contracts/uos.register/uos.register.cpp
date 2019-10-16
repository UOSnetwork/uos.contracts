#include "uos.register.hpp"

namespace UOS {

    void uos_register::regname(name eos_account, name uos_account) {
        require_auth(eos_account);

        accregister_table ar_table(_self, eos_account.value);
        auto itr = ar_table.find(eos_account.value);
        check(itr == ar_table.end(), "account name already registered");
        ar_table.emplace(eos_account, [&](account_register &item) {
                item.eos_account = eos_account;
                item.uos_account = uos_account;
            });
    }

    EOSIO_DISPATCH( uos_register, (regname))
}