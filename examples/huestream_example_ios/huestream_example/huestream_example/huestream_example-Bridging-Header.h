//
//  huestream_example-Bridging-Header.h
//  huestream_example
//  Copyright © 2021 Signify. All rights reserved.
//
//
//  Use this file to import your target's public headers that you would like to expose to Swift.
//
#import <Foundation/Foundation.h>

// These enum are both copies of the one in the edk Feedback.h since we can't include that file here.
typedef enum {
		FEEDBACK_TYPE_INFO,   ///< general progress information
		FEEDBACK_TYPE_USER    ///< action from the user is required
} HueFeedbackType;

typedef enum {
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
		ID_INTERNAL_ERROR                          ///< should not happen on a production system
} HueFeedbackId;

@interface HueStreamFeedbackMessage : NSObject
@property HueFeedbackId feedbackId;
@property HueFeedbackType feedbackType;
@property NSString* userMessage;
@end

@interface HueStreamWrapper : NSObject
-(void)setHueMessageHandler:(void(^)(HueStreamFeedbackMessage*)) handler;
-(void)connect;
-(void) abortConnect;
-(NSString*)getBridgeName;
-(NSMutableArray*)getEntertainmentAreaList;
-(int)getSelectedEntertainmentArea;
-(void)useEntertainmentArea:(int) position;
-(void)newHueMessage:(HueStreamFeedbackMessage*) msg;
-(void)setLightsToGreen;
-(void)showAnimatedLightExample;
-(void)resetHueData;
@end
