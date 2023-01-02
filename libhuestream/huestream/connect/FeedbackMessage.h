/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_CONNECT_FEEDBACKMESSAGE_H_
#define HUESTREAM_CONNECT_FEEDBACKMESSAGE_H_

#include "huestream/common/data/Bridge.h"
#include "huestream/common/language/IMessageTranslator.h"

#include <memory>
#include <string>
#include <functional>
#include <map>

namespace huestream {

        /**
         class used to give feedback from the library to the application, mainly on status of bridge connection
         @see doc/ApplicationNote_HueEDK_ConnectionFlow.pdf
         */
        class FeedbackMessage {
        public:
            /**
             feedback message identifier
             */
            enum Id {
                ID_USERPROCEDURE_STARTED,                  ///< connection procedure started
                ID_START_LOADING,
                ID_FINISH_LOADING_NO_BRIDGE_CONFIGURED,
                ID_FINISH_LOADING_BRIDGE_CONFIGURED,
                ID_START_SEARCHING,
                ID_FINISH_SEARCHING_NO_BRIDGES_FOUND,
                ID_FINISH_SEARCHING_INVALID_BRIDGES_FOUND,
                ID_FINISH_SEARCH_BRIDGES_FOUND,
                ID_START_AUTHORIZING,
                ID_PRESS_PUSH_LINK,                        ///< user should press the pushlink button on the bridge
                ID_FINISH_AUTHORIZING_AUTHORIZED,
                ID_FINISH_AUTHORIZING_FAILED,
                ID_START_RETRIEVING_SMALL,
                ID_FINISH_RETRIEVING_SMALL,
                ID_START_RETRIEVING,
                ID_FINISH_RETRIEVING_FAILED,
                ID_FINISH_RETRIEVING_READY_TO_START,
                ID_FINISH_RETRIEVING_ACTION_REQUIRED,
                ID_START_SAVING,
                ID_FINISH_SAVING_SAVED,
                ID_FINISH_SAVING_FAILED,
                ID_START_ACTIVATING,
                ID_FINISH_ACTIVATING_ACTIVE,
                ID_FINISH_ACTIVATING_FAILED,
                ID_NO_BRIDGE_FOUND,                        ///< no bridge was found on the network
                ID_BRIDGE_NOT_FOUND,                       ///< the configured bridge was not found on the network
                ID_INVALID_MODEL,                          ///< bridge hardware does not support streaming
                ID_INVALID_VERSION,                        ///< bridge verison does not support streaming
                ID_NO_GROUP_AVAILABLE,                     ///< no entertainment group is configured on the bridge
                ID_BUSY_STREAMING,                         ///< another streaming session is ongoing on the bridge
                ID_SELECT_GROUP,                           ///< there are multiple entertainment groups and none is selected yet
                ID_DONE_NO_BRIDGE_FOUND,                   ///< connection request has finished without finding any bridges
                ID_DONE_BRIDGE_FOUND,                      ///< connection request has discovered a bridge which can be connected to
                ID_DONE_ACTION_REQUIRED,                   ///< connection request has interrupted because a user action is required
                ID_DONE_COMPLETED,                         ///< connection request has been completed
                ID_DONE_ABORTED,                           ///< connection procedure has been aborted
                ID_DONE_RESET,                             ///< reset request has completed
                ID_USERPROCEDURE_FINISHED,                 ///< connection procedure ended
                ID_BRIDGE_CONNECTED,                       ///< bridge got connected
                ID_BRIDGE_DISCONNECTED,                    ///< bridge got disconnected
                ID_STREAMING_CONNECTED,                    ///< streaming got connected
                ID_STREAMING_DISCONNECTED,                 ///< streaming got disconnected
                ID_BRIDGE_CHANGED,                         ///< bridge config changed (id, name, version etc)
                ID_LIGHTS_UPDATED,                         ///< lights in the group have changed
                ID_GROUPLIST_UPDATED,                      ///< list of entertainment groups has changed
                ID_GROUP_LIGHTSTATE_UPDATED,               ///< home automation light state of the group has changed
                ID_ZONELIST_UPDATED,                       ///< list of zones has changed
                ID_ZONE_SCENELIST_UPDATED,
                ID_INVALID_EVENTING_CONNECTION,
                ID_BRIDGE_REFRESHED,
                ID_INTERNAL_ERROR                          ///< should not happen on a production system
            };

            /**
             type of requests supported by connection flow
             */
            enum RequestType {
                REQUEST_TYPE_INTERNAL,            ///< no request
                REQUEST_TYPE_LOAD_BRIDGE,         ///< load stored bridge information
                REQUEST_TYPE_CONNECT,             ///< if a valid bridge is stored connect to it, else discover a new bridge and connect to it
                REQUEST_TYPE_CONNECT_BACKGROUND,  ///< if a valid bridge is stored connect to it, else check if a valid bridge is on the network but don't connect yet
                REQUEST_TYPE_CONNECT_IP,          ///< connect to a bridge with manually set ip address
                REQUEST_TYPE_SET_MANUAL_BRIDGE,   ///< connect to a known bridge, with at least set ip address, username and clientkey
                REQUEST_TYPE_CONNECT_NEW,         ///< connect to a new bridge, even if a valid bridge is stored already
                REQUEST_TYPE_CONNECT_ID_KEY,      ///< connect to bridge with manually set bridge id, username and clientkey
                REQUEST_TYPE_RESET_BRIDGE,        ///< delete stored information of current bridge
                REQUEST_TYPE_RESET_ALL,           ///< delete all stored bridge information
                REQUEST_TYPE_SELECT,              ///< select entertainment group to use
                REQUEST_TYPE_ACTIVATE,            ///< start streaming
                REQUEST_TYPE_DEACTIVATE           ///< stop streaming
            };

            /**
             type of feedback message
             */
            enum FeedbackType {
                FEEDBACK_TYPE_INFO,   ///< general progress information
                FEEDBACK_TYPE_USER    ///< action from the user is required
            };

            /**
             constructor
             @param reqType Type of request this feedback originates from
             @param id Identifier of the feedback message
             @param bridge Reference to object holding current state of loaded bridge
             */
            FeedbackMessage(RequestType reqType, Id id, BridgePtr bridge);

            /**
             constructor
             @param reqType Type of request this feedback originates from
             @param id Identifier of the feedback message
             @param bridge Reference to object holding current state of loaded bridge
             @param bridge Reference to list of bridges
             */
            FeedbackMessage(FeedbackMessage::RequestType reqType, Id id, BridgePtr bridge, BridgeListPtr bridgeList);

            /**
             destructor
             */
            virtual ~FeedbackMessage();

            /**
             get type of feedback message
             @return wtether this is feedback requires a USER action or is other INFO
             */
            FeedbackType GetMessageType() const;

            /**
             get tag (or ‘string id’) used for looking up the user message translation
             @return tag string
             */
            std::string GetTag() const;

            /**
             If the message type is USER, get a user message string explaining the user what to do
             @return user message string in the language the EDK is configured in
             */
            std::string GetUserMessage() const;

            /**
             get a string to output to debug logging
             @return debug string describing feedback message
             */
            std::string GetDebugMessage() const;

            static void SetTranslator(MessageTranslatorPtr translator);

        protected:
            static std::map<Id, FeedbackType> idTypeMap;
            static std::map<Id, std::string> idTagMap;
            static std::map<FeedbackMessage::RequestType, std::string> requestStringMap;
            static MessageTranslatorPtr _translator;

        PROP_DEFINE(FeedbackMessage, Id, id, Id);
        PROP_DEFINE(FeedbackMessage, RequestType, requestType, RequestType);
        PROP_DEFINE(FeedbackMessage, BridgePtr, bridge, Bridge);
        PROP_DEFINE(FeedbackMessage, BridgeListPtr, bridgeList, BridgeList);
        };

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_FEEDBACKMESSAGE_H_
