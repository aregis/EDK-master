/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/config/Config.h>
#include <huestream/connect/FeedbackMessage.h>

#include <string>
#include <map>

namespace huestream {

MessageTranslatorPtr FeedbackMessage::_translator;

FeedbackMessage::FeedbackMessage(FeedbackMessage::RequestType reqType, Id id, BridgePtr bridge) :
    _id(id), _requestType(reqType), _bridge(bridge), _bridgeList(std::make_shared<BridgeList>()) {
}

FeedbackMessage::FeedbackMessage(FeedbackMessage::RequestType reqType, Id id, BridgePtr bridge, BridgeListPtr bridgeList) :
    _id(id), _requestType(reqType), _bridge(bridge), _bridgeList(bridgeList) {
}

FeedbackMessage::~FeedbackMessage() {
}

std::map<FeedbackMessage::Id, FeedbackMessage::FeedbackType> FeedbackMessage::idTypeMap = {
    {ID_USERPROCEDURE_STARTED,                  FEEDBACK_TYPE_INFO},
    {ID_START_LOADING,                          FEEDBACK_TYPE_INFO},
    {ID_FINISH_LOADING_NO_BRIDGE_CONFIGURED,    FEEDBACK_TYPE_INFO},
    {ID_FINISH_LOADING_BRIDGE_CONFIGURED,       FEEDBACK_TYPE_INFO},
    {ID_START_SEARCHING,                        FEEDBACK_TYPE_INFO},
    {ID_FINISH_SEARCHING_NO_BRIDGES_FOUND,      FEEDBACK_TYPE_INFO},
    {ID_FINISH_SEARCHING_INVALID_BRIDGES_FOUND, FEEDBACK_TYPE_INFO},
    {ID_FINISH_SEARCH_BRIDGES_FOUND,            FEEDBACK_TYPE_INFO},
    {ID_START_AUTHORIZING,                      FEEDBACK_TYPE_INFO},
    {ID_PRESS_PUSH_LINK,                        FEEDBACK_TYPE_USER},
    {ID_FINISH_AUTHORIZING_AUTHORIZED,          FEEDBACK_TYPE_INFO},
    {ID_FINISH_AUTHORIZING_FAILED,              FEEDBACK_TYPE_INFO},
    {ID_START_RETRIEVING_SMALL,                 FEEDBACK_TYPE_INFO},
    {ID_FINISH_RETRIEVING_SMALL,                FEEDBACK_TYPE_INFO},
    {ID_START_RETRIEVING,                       FEEDBACK_TYPE_INFO},
    {ID_FINISH_RETRIEVING_FAILED,               FEEDBACK_TYPE_INFO},
    {ID_FINISH_RETRIEVING_READY_TO_START,       FEEDBACK_TYPE_INFO},
    {ID_FINISH_RETRIEVING_ACTION_REQUIRED,      FEEDBACK_TYPE_INFO},
    {ID_START_SAVING,                           FEEDBACK_TYPE_INFO},
    {ID_FINISH_SAVING_SAVED,                    FEEDBACK_TYPE_INFO},
    {ID_FINISH_SAVING_FAILED,                   FEEDBACK_TYPE_INFO},
    {ID_START_ACTIVATING,                       FEEDBACK_TYPE_INFO},
    {ID_FINISH_ACTIVATING_ACTIVE,               FEEDBACK_TYPE_INFO},
    {ID_FINISH_ACTIVATING_FAILED,               FEEDBACK_TYPE_INFO},
    {ID_NO_BRIDGE_FOUND,                        FEEDBACK_TYPE_USER},
    {ID_BRIDGE_NOT_FOUND,                       FEEDBACK_TYPE_USER},
    {ID_INVALID_MODEL,                          FEEDBACK_TYPE_USER},
    {ID_INVALID_VERSION,                        FEEDBACK_TYPE_USER},
    {ID_NO_GROUP_AVAILABLE,                     FEEDBACK_TYPE_USER},
    {ID_BUSY_STREAMING,                         FEEDBACK_TYPE_USER},
    {ID_SELECT_GROUP,                           FEEDBACK_TYPE_USER},
    {ID_DONE_NO_BRIDGE_FOUND,                   FEEDBACK_TYPE_INFO},
    {ID_DONE_BRIDGE_FOUND,                      FEEDBACK_TYPE_INFO},
    {ID_DONE_ACTION_REQUIRED,                   FEEDBACK_TYPE_INFO},
    {ID_DONE_COMPLETED,                         FEEDBACK_TYPE_INFO},
    {ID_DONE_ABORTED,                           FEEDBACK_TYPE_INFO},
    {ID_DONE_RESET,                             FEEDBACK_TYPE_INFO},
    {ID_USERPROCEDURE_FINISHED,                 FEEDBACK_TYPE_INFO},
    {ID_BRIDGE_CONNECTED,                       FEEDBACK_TYPE_INFO},
    {ID_BRIDGE_DISCONNECTED,                    FEEDBACK_TYPE_INFO},
    {ID_STREAMING_CONNECTED,                    FEEDBACK_TYPE_INFO},
    {ID_STREAMING_DISCONNECTED,                 FEEDBACK_TYPE_USER},
    {ID_BRIDGE_CHANGED,                         FEEDBACK_TYPE_INFO},
    {ID_LIGHTS_UPDATED,                         FEEDBACK_TYPE_INFO},
    {ID_GROUPLIST_UPDATED,                      FEEDBACK_TYPE_INFO},
    {ID_GROUP_LIGHTSTATE_UPDATED,               FEEDBACK_TYPE_INFO},
    {ID_ZONELIST_UPDATED,                       FEEDBACK_TYPE_INFO},
    {ID_ZONE_SCENELIST_UPDATED,                 FEEDBACK_TYPE_INFO},
    {ID_INVALID_EVENTING_CONNECTION,            FEEDBACK_TYPE_INFO},
    {ID_BRIDGE_REFRESHED,                       FEEDBACK_TYPE_INFO},
    {ID_INTERNAL_ERROR,                         FEEDBACK_TYPE_USER}
};

std::map<FeedbackMessage::Id, std::string> FeedbackMessage::idTagMap = {
    {ID_USERPROCEDURE_STARTED,                  "ID_USERPROCEDURE_STARTED"},
    {ID_START_LOADING,                          "ID_START_LOADING"},
    {ID_FINISH_LOADING_NO_BRIDGE_CONFIGURED,    "ID_FINISH_LOADING_NO_BRIDGE_CONFIGURED"},
    {ID_FINISH_LOADING_BRIDGE_CONFIGURED,       "ID_FINISH_LOADING_BRIDGE_CONFIGURED"},
    {ID_START_SEARCHING,                        "ID_START_SEARCHING"},
    {ID_FINISH_SEARCHING_NO_BRIDGES_FOUND,      "ID_FINISH_SEARCHING_NO_BRIDGES_FOUND"},
    {ID_FINISH_SEARCHING_INVALID_BRIDGES_FOUND, "ID_FINISH_SEARCHING_INVALID_BRIDGES_FOUND"},
    {ID_FINISH_SEARCH_BRIDGES_FOUND,            "ID_FINISH_SEARCH_BRIDGES_FOUND"},
    {ID_START_AUTHORIZING,                      "ID_START_AUTHORIZING"},
    {ID_PRESS_PUSH_LINK,                        "ID_PRESS_PUSH_LINK"},
    {ID_FINISH_AUTHORIZING_AUTHORIZED,          "ID_FINISH_AUTHORIZING_AUTHORIZED"},
    {ID_FINISH_AUTHORIZING_FAILED,              "ID_FINISH_AUTHORIZING_FAILED"},
    {ID_START_RETRIEVING_SMALL,                 "ID_START_RETRIEVING_SMALL"},
    {ID_FINISH_RETRIEVING_SMALL,                "ID_FINISH_RETRIEVING_SMALL"},
    {ID_START_RETRIEVING,                       "ID_START_RETRIEVING"},
    {ID_FINISH_RETRIEVING_FAILED,               "ID_FINISH_RETRIEVING_FAILED"},
    {ID_FINISH_RETRIEVING_READY_TO_START,       "ID_FINISH_RETRIEVING_READY_TO_START"},
    {ID_FINISH_RETRIEVING_ACTION_REQUIRED,      "ID_FINISH_RETRIEVING_ACTION_REQUIRED"},
    {ID_START_SAVING,                           "ID_START_SAVING"},
    {ID_FINISH_SAVING_SAVED,                    "ID_FINISH_SAVING_SAVED"},
    {ID_FINISH_SAVING_FAILED,                   "ID_FINISH_SAVING_FAILED"},
    {ID_START_ACTIVATING,                       "ID_START_ACTIVATING"},
    {ID_FINISH_ACTIVATING_ACTIVE,               "ID_FINISH_ACTIVATING_ACTIVE"},
    {ID_FINISH_ACTIVATING_FAILED,               "ID_FINISH_ACTIVATING_FAILED"},
    {ID_NO_BRIDGE_FOUND,                        "ID_NO_BRIDGE_FOUND"},
    {ID_BRIDGE_NOT_FOUND,                       "ID_BRIDGE_NOT_FOUND"},
    {ID_INVALID_MODEL,                          "ID_INVALID_MODEL"},
    {ID_INVALID_VERSION,                        "ID_INVALID_VERSION"},
    {ID_NO_GROUP_AVAILABLE,                     "ID_NO_GROUP_AVAILABLE"},
    {ID_BUSY_STREAMING,                         "ID_BRIDGE_BUSY"},
    {ID_SELECT_GROUP,                           "ID_SELECT_GROUP"},
    {ID_DONE_NO_BRIDGE_FOUND,                   "ID_DONE_NO_BRIDGE_FOUND"},
    {ID_DONE_BRIDGE_FOUND,                      "ID_DONE_BRIDGE_FOUND"},
    {ID_DONE_ACTION_REQUIRED,                   "ID_DONE_ACTION_REQUIRED"},
    {ID_DONE_COMPLETED,                         "ID_DONE_COMPLETED"},
    {ID_DONE_ABORTED,                           "ID_DONE_ABORTED"},
    {ID_DONE_RESET,                             "ID_DONE_RESET" },
    {ID_USERPROCEDURE_FINISHED,                 "ID_USERPROCEDURE_FINISHED"},
    {ID_BRIDGE_CONNECTED,                       "ID_BRIDGE_CONNECTED"},
    {ID_BRIDGE_DISCONNECTED,                    "ID_BRIDGE_DISCONNECTED"},
    {ID_STREAMING_CONNECTED,                    "ID_STREAMING_CONNECTED"},
    {ID_STREAMING_DISCONNECTED,                 "ID_STREAMING_DISCONNECTED"},
    {ID_BRIDGE_CHANGED,                         "ID_BRIDGE_CHANGED"},
    {ID_LIGHTS_UPDATED,                         "ID_LIGHTS_UPDATED"},
    {ID_GROUPLIST_UPDATED,                      "ID_GROUPLIST_UPDATED"},
    {ID_GROUP_LIGHTSTATE_UPDATED,               "ID_GROUP_LIGHTSTATE_UPDATED"},
    {ID_ZONELIST_UPDATED,                       "ID_ZONELIST_UPDATED"},
    {ID_ZONE_SCENELIST_UPDATED,                 "ID_ZONE_SCENELIST_UPDATED"},
    {ID_INVALID_EVENTING_CONNECTION,            "ID_INVALID_EVENTING_CONNECTION"},
    {ID_BRIDGE_REFRESHED,                       "ID_BRIDGE_REFRESHED"},
    {ID_INTERNAL_ERROR,                         "ID_INTERNAL_ERROR"}
};

std::map<FeedbackMessage::RequestType, std::string> FeedbackMessage::requestStringMap = {
    {REQUEST_TYPE_INTERNAL,           "REQUEST_TYPE_INTERNAL"},
    {REQUEST_TYPE_LOAD_BRIDGE,        "REQUEST_TYPE_LOAD_BRIDGE"},
    {REQUEST_TYPE_CONNECT,            "REQUEST_TYPE_CONNECT"},
    {REQUEST_TYPE_CONNECT_BACKGROUND, "REQUEST_TYPE_CONNECT_BACKGROUND"},
    {REQUEST_TYPE_CONNECT_IP,         "REQUEST_TYPE_CONNECT_IP"},
    {REQUEST_TYPE_CONNECT_NEW,        "REQUEST_TYPE_CONNECT_NEW"},
    {REQUEST_TYPE_SET_MANUAL_BRIDGE,  "REQUEST_TYPE_SET_MANUAL_BRIDGE"},
    {REQUEST_TYPE_CONNECT_ID_KEY,     "REQUEST_TYPE_CONNECT_ID_KEY"},
    {REQUEST_TYPE_RESET_BRIDGE,       "REQUEST_TYPE_RESET_BRIDGE"},
    {REQUEST_TYPE_RESET_ALL,          "REQUEST_TYPE_RESET_ALL"},
    {REQUEST_TYPE_SELECT,             "REQUEST_TYPE_SELECT"},
    {REQUEST_TYPE_ACTIVATE,           "REQUEST_TYPE_ACTIVATE"},
    {REQUEST_TYPE_DEACTIVATE,         "REQUEST_TYPE_DEACTIVATE"}
};

PROP_IMPL(FeedbackMessage, FeedbackMessage::Id, id, Id);
PROP_IMPL(FeedbackMessage, FeedbackMessage::RequestType, requestType, RequestType);
PROP_IMPL(FeedbackMessage, BridgePtr, bridge, Bridge);
PROP_IMPL(FeedbackMessage, BridgeListPtr, bridgeList, BridgeList);

FeedbackMessage::FeedbackType FeedbackMessage::GetMessageType() const {
    auto it = idTypeMap.find(_id);
    if (it != idTypeMap.end())
        return it->second;
    return FeedbackMessage::FEEDBACK_TYPE_INFO;
}

std::string FeedbackMessage::GetTag() const {
    auto it = idTagMap.find(_id);
    if (it != idTagMap.end())
        return it->second;
    return "UNKOWN";
}

std::string FeedbackMessage::GetUserMessage() const {
    std::string userMessage;
    if (_translator) {
        userMessage = _translator->Translate(GetTag());
    }
    if (_id == ID_PRESS_PUSH_LINK) {
        auto numBridges = GetBridgeList()->size();
        userMessage = std::to_string(numBridges) + " Hue Bridge" + (numBridges == 1 ? "" : "s") + userMessage;
    }
    auto groupsOwnedByOtherClient = _bridge->GetGroupsOwnedByOtherClient();
    if (_id == ID_BUSY_STREAMING && groupsOwnedByOtherClient->size() > 0) {
        userMessage = userMessage + groupsOwnedByOtherClient->at(0)->GetFriendlyOwnerName() + " - " +
            groupsOwnedByOtherClient->at(0)->GetName() + ".";
    }
    return userMessage;
}

std::string FeedbackMessage::GetDebugMessage() const {
    auto requestString = std::string("Unknown");
    auto it = requestStringMap.find(_requestType);
    if (it != requestStringMap.end())
        requestString = it->second;

    return requestString + ": " + GetTag();
}

void FeedbackMessage::SetTranslator(MessageTranslatorPtr translator) {
    _translator = translator;
}
}  // namespace huestream
