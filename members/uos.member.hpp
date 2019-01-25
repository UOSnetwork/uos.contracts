#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <string>

namespace UOS {
    using namespace eosio;
    using std::string;

    class Members : public contract {
        using contract::contract;

        public:

            Members(account_name self):contract(self) {}

            //@abi action
            void add(account_name account, string& username);

            //@abi action
            void update(account_name account, uint64_t level);

            //@abi action
            void getmember(const account_name account);

            //@abi action
            void addability(const account_name account, string& ability);

            //@abi table item i64
            struct item {
                uint64_t item_id;
                string name;
                string ability;
                uint64_t level_up;

                uint64_t primary_key() const { return item_id; }

                EOSLIB_SERIALIZE(item, (item_id)(name)(ability)(level_up))
            };

            //@abi action
            void additem(const account_name account, item purchased_item);

            //@abi table member i64
            struct member {
                uint64_t account_name;
                string username;
                uint64_t level;
                vector<string> abilities;
                vector<item> inventory;

                uint64_t primary_key() const { return account_name; }

                EOSLIB_SERIALIZE(member, (account_name)(username)(level)(abilities)(inventory))
            };

            typedef multi_index<N(member), member> memberIndex;
    };

    EOSIO_ABI(Members, (add)(update)(getmember)(addability)(additem));
}
