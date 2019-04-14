#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/stdlib.hpp>
#include <algorithm>

#include <string>

namespace uos {
    using namespace eosio;
    using std::string;
    using std::vector;

    class uos_docs : public contract {
    public:
        uos_docs(account_name self) : contract(self) {}

    private:
        struct attachment_history_object {
            time datetime;
            string uri;
            account_name account;
            string state;
        };
        struct attachment_record {
            uint8_t idx;
            string description;
            vector <attachment_history_object> attachment_history;
        };
        struct document_state_object {
            account_name account;
            string message;
            string state;
        };
        struct document_history_record {
            time date_of_record;
            document_state_object document_state;
        };

        struct route_record{
            account_name from;
            account_name to;
            string action;
        };

        /**
          *@brief   card of documents
          *
          */
        //@abi table document i64
        struct document {
            uint64_t uuid;
            string descriptor;
            vector <document_history_record> document_history;
            vector <attachment_record> attachments;
            vector <route_record> document_route;

            uint64_t primary_key() const { return uuid; }

            EOSLIB_SERIALIZE(document, (uuid)(descriptor)(document_history)(attachments)(document_route))
        };


        //TODO:rename table doclist
        //@abi table user_ifiles i64
        struct user_ifiles {
            uint64_t uuid;
            uint64_t primary_key() const { return uuid; }

            EOSLIB_SERIALIZE(user_ifiles, (uuid))
        };

        typedef multi_index<N(user),user_ifiles> docUserIndex;

        typedef multi_index<N(document), document> docsIndex;

    public:
        //@abi action
        void createdoc(const uint64_t uuid,  string descriptor, vector <route_record> route,const account_name account);

        //@abi action
        void execute(const uint64_t uuid, string action, const account_name account);

        /**
         *
         *@brief step of route documents (history add owner documents)
         *
         */
         //TODO:check owner documents within transit route,so will make edit route on index_array;
        //@abi action
        void route(const uint64_t uuid, string action, uint8_t index);

        //@abi action
        void attach(const uint64_t uuid, string descriptor, string uri,const account_name account);

        /**
         * @brief so version to documents
         *
         */
        //@abi action
        void update(const uint64_t uuid, string uri,uint8_t attachment_idx, account_name account);

    protected:
        /**
         * @brief change route doc
        */

        //TODO: so type variable's  state uint8_t or string

        void editroute(uint64_t uuid,const account_name acc_from, const account_name acc_to, uint8_t state );

        void addhistory(uint64_t uuid, document_history_record &history);

        void attachments_doc(uint64_t uuid, string description, attachment_history_object &attachments );

    };
    EOSIO_ABI(uos_docs, (createdoc)(execute)(route)(attach)(update))
}
