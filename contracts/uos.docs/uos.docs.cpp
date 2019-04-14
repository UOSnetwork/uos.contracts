#include "uos.docs.hpp"
#include <eosiolib/system.h>

namespace uos {

    void uos_docs::createdoc(const uint64_t uuid, std::string descriptor,vector<route_record> route, const account_name account)
    {
        //require_auth( _self );
        //require_recipient(_self);


        print("Create document start","\n");
        docsIndex documents_list(_self,_self);

        auto document = documents_list.find(uuid);
        print(document==documents_list.end(),"\n");
        eosio_assert(document==documents_list.end(),"Document is already created");

        std::set<account_name> actors;

        for(auto item: route){
            actors.insert(item.from);
            actors.insert(item.to);
        }

        for(auto item: actors){
            eosio_assert(is_account(item),"Account from route is not created. Create it before first using");
        }



        documents_list.emplace(_self,[&](auto &record){
            record.uuid = uuid;
            record.descriptor = descriptor;
            record.document_route = std::move(route);
        });
        print("Record created","\n");

        for(auto item: actors){
            print(name{item},"\n");
            docUserIndex doc_user(_self,item);
            auto record = doc_user.find(uuid);

            if(record==doc_user.end()){
                doc_user.emplace(_self,[&](auto &rec){
                    rec.uuid = uuid;
                });
            }
        }
        document_history_record state;
        state.document_state.account = account;
        state.document_state.message = "creation";
        state.document_state.state = "created";
        state.date_of_record = now();
        addhistory(uuid,state);

    }

    void uos_docs::editroute(uint64_t uuid,const account_name acc_from, const account_name acc_to, uint8_t state )
    {
        print("Edit route start\n");
    }

    void uos_docs::addhistory(uint64_t uuid, document_history_record &history)
    {
        print("Add history start\n");
        docsIndex documents_list(_self,_self);
        auto doc = documents_list.find(uuid);
        eosio_assert(doc!=documents_list.end(),"Document is uuid not found");

        documents_list.modify(doc,0,[&](document & item){
            item.document_history.push_back(history);
        });

    }

    void uos_docs::attachments_doc(uint64_t uuid, string description, attachment_history_object &attachments)
    {
        print("Attachment doc start\n");
        docsIndex documents_list(_self,_self);

        auto doc = documents_list.find(uuid);
        eosio_assert(doc!=documents_list.end(),"Document is uuid not found");

        attachment_record hist_file;
        hist_file.description = description;
        hist_file.attachment_history.push_back(attachments);

        documents_list.modify(doc,0,[&](document & item){
            hist_file.idx = item.attachments.size()+1;
            item.attachments.push_back(hist_file);
        });

        document_history_record state;
        state.document_state.account = attachments.account;
        state.document_state.message = "attachments_docs";
        state.document_state.state = "attaches";
        state.date_of_record = now();
        addhistory(uuid,state);

    }

    void uos_docs::execute(const uint64_t uuid, std::string action, const account_name account)
    {
        print("Execute action\n");

        document_history_record state;
        state.document_state.account = account;
        state.document_state.message = "route_in_action";
        state.document_state.state = action;
        state.date_of_record = now();
        addhistory(uuid,state);
    }

    void uos_docs::attach(const uint64_t uuid, std::string descriptor, std::string uri,const account_name account)
    {
        print("Attach uri");

        attachment_history_object attach_hist_file;
        attach_hist_file.account = account;
        attach_hist_file.datetime = now();
        attach_hist_file.state = "attach";
        attach_hist_file.uri = uri;
        attachments_doc(uuid,descriptor,attach_hist_file);
    }


    void uos_docs::update(const uint64_t uuid, std::string uri, uint8_t attachment_idx,account_name account)
    {
        print("Update document");

        attachment_history_object  attach_hist_file;
        attach_hist_file.account = account;
        attach_hist_file.datetime = now();
        attach_hist_file.state = "update version document";
        attach_hist_file.uri = uri;

        print("Attachment doc history start\n");
        docsIndex documents_list(_self,_self);

        auto doc = documents_list.find(uuid);
        eosio_assert(doc!=documents_list.end(),"Document is uuid not found");

        documents_list.modify(doc,0,[&](document & item){

            for(auto & att_rec : item.attachments)
            {
                if(att_rec.idx == attachment_idx) {
                    print("Found element with idx");
                    att_rec.attachment_history.push_back(attach_hist_file);
                }
            }
        });

        document_history_record state;
        state.document_state.account = account;
        state.document_state.message = "new_version_attachments_docs";
        state.document_state.state = "attaches";
        state.date_of_record = now();
        addhistory(uuid,state);
    }

    void uos_docs::route(const uint64_t uuid, std::string action, uint8_t index)
    {
        print("Start edit route ","\n");
        docsIndex docs (_self,_self);

        auto doc = docs.find(uuid);
        eosio_assert(doc != docs.end(),"Document is uuid not found");

        docs.modify(doc,0,[&](document & item){

            auto r_size = item.document_route.size();
            print( "Route manchmal Größenreferenz:");
            print (r_size, "\n");
            print ((int)index, "\n");
//            eosio_assert(index >(int)r_size,"Index can't more than size ");
            print(item.document_route.at(index).from, item.document_route.at(index).to,item.document_route.at(index).action,"\n");
            item.document_route.at(index).action = action;

        });

        //addhistory(); edit route:exchange action for one's action
    }

}
