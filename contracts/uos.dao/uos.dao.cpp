#include "uos.dao.hpp"

namespace UOS {
    void uos_dao::setparam(std::string name, std::string value) {
        print("NAME ", name, '\n');
        print("VALUE ", value, '\n');

        params_table p_table(_self, _self.value);

        auto itr = p_table.begin();
        for(; itr != p_table.end(); itr++){
            print("PARAM ", itr->name, " ", itr->value, "\n");
            if(itr->name == name) {
                print("FOUND\n");
                break;
            }
        }

        if(itr == p_table.end()) {
            print("CREATE PARAM");
            p_table.emplace(_self, [&](param &p) {
                p.id = p_table.available_primary_key();
                p.name = name;
                p.value = value;
            });
        } else {
            print("MODIFY PARAM");
            p_table.modify(itr,_self,[&](param& p){
               p.value = value;
            });
        }
    }

    EOSIO_DISPATCH( uos_dao, (setparam))

}
