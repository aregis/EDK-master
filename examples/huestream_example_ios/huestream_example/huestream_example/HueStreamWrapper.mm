//
//  HueStreamWrapper.mm
//  huestream_example
//  Copyright © 2021 Signify. All rights reserved.
//

#import "huestream_example-Bridging-Header.h"
#include "huestream/HueStream.h"
#include "huestream/common/serialize/SerializerHelper.h"
#include "huestream/config/Config.h"
#include "huestream/connect/IFeedbackMessageHandler.h"
#include "huestream/connect/FeedbackMessage.h"
#include "huestream/effect/effects/AreaEffect.h"
#include "huestream/effect/effects/LightSourceEffect.h"
#include "huestream/effect/effects/ExplosionEffect.h"
#include "huestream/effect/animation/animations/ConstantAnimation.h"
#include "huestream/effect/animation/animations/CurveAnimation.h"
#include "huestream/effect/animation/data/Point.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

using namespace huestream;

class MyFeedbackHandler : public IFeedbackMessageHandler {
	public:
		MyFeedbackHandler() = default;
		MyFeedbackHandler(HueStreamWrapper* const internal) {
			mInternal = CFBridgingRetain(internal);
		}
		virtual ~MyFeedbackHandler() = default;
	
		virtual void NewFeedbackMessage(const FeedbackMessage &message) const {
			printf("%s\n", message.GetDebugMessage().c_str());
			
			std::shared_ptr<CurveAnimation> sawTooth = std::shared_ptr<CurveAnimation>(new CurveAnimation(5.0, nullptr));
			
			@autoreleasepool {
				HueStreamWrapper* const internal = (__bridge HueStreamWrapper*) mInternal;
				
				HueStreamFeedbackMessage* hueFeedbackMessage = [[HueStreamFeedbackMessage alloc] init];
				hueFeedbackMessage.feedbackId = (HueFeedbackId)message.GetId();
				hueFeedbackMessage.feedbackType = (HueFeedbackType)message.GetMessageType();
				
				if (message.GetMessageType() == FeedbackMessage::FeedbackType::FEEDBACK_TYPE_USER) {
					CFStringRef msg = CFStringCreateWithCString(kCFAllocatorDefault, message.GetUserMessage().c_str(), kCFStringEncodingASCII);
					hueFeedbackMessage.userMessage = (__bridge NSString*)msg;
				}
				
				[internal newHueMessage:hueFeedbackMessage];
			}
		}
	
	private:
		const void* mInternal = nullptr;
};

@interface HueStreamWrapper()
@property std::shared_ptr<HueStream> huestream;
@property std::shared_ptr<MyFeedbackHandler> feedbackHandler;
@property void (^hueMessageHandler)(HueStreamFeedbackMessage*);
@property std::shared_ptr<AreaEffect> greenEffect;
@property std::shared_ptr<AreaEffect> leftEffect;
@property std::shared_ptr<LightSourceEffect> rightEffect;
@property std::shared_ptr<ExplosionEffect> explosionEffect;
@end

@implementation HueStreamWrapper
-(id) init {
	if (self = [super init]) {
		UIDevice* deviceInfo = [UIDevice currentDevice];
		NSString* deviceName =	deviceInfo.name;
		
		auto hueConfig = std::make_shared<huestream::Config>("Dummy Example", deviceName.UTF8String, PersistenceEncryptionKey{"nfDk3l7rMw9c"});
		hueConfig->GetAppSettings()->SetAutoStartAtConnection(false);
		// Can only write files in the Documents folder on iOS
		std::string path = getenv("HOME");
		path += "/Documents/bridge.json";
		hueConfig->GetAppSettings()->SetBridgeFileName(path);
		
		self.huestream = std::make_shared<huestream::HueStream>(hueConfig);
		self.feedbackHandler = std::make_shared<MyFeedbackHandler>(self);
		self.huestream->RegisterFeedbackHandler(self.feedbackHandler);
	}
	
	return self;
}

-(void) setHueMessgeHandler:(void(^)(HueStreamFeedbackMessage*)) handler {
	_hueMessageHandler = handler;
}

-(void) connect {
	self.huestream->ConnectBridgeAsync();
}

-(void) abortConnect {
	self.huestream->AbortConnecting();
}

-(NSString*)getBridgeName {
	CFStringRef bridgeName = CFStringCreateWithCString(kCFAllocatorDefault, self.huestream->GetActiveBridge()->GetName().c_str(), kCFStringEncodingASCII);
	return (__bridge NSString*)bridgeName;
}

-(NSMutableArray*)getEntertainmentAreaList {
	auto groupList = self.huestream->GetActiveBridge()->GetGroups();
	NSMutableArray* areaList = [NSMutableArray arrayWithCapacity:groupList->size()];
	
	for (int i = 0 ; i < groupList->size(); ++i) {
		auto group = groupList->at(i);
		CFStringRef areaName = CFStringCreateWithCString(kCFAllocatorDefault, group->GetName().c_str(), kCFStringEncodingASCII);
		[areaList insertObject:(__bridge NSString*)areaName atIndex:i];
	}
	
	return areaList;
}

-(int)getSelectedEntertainmentArea {
	auto bridge = self.huestream->GetActiveBridge();
	
	if (!bridge->IsValidGroupSelected()) {
		return -1;
	}
	
	auto groupList = bridge->GetGroups();
	auto selectedGroup = self.huestream->GetActiveBridge()->GetGroup();
	
	for (int i = 0 ; i < groupList->size(); ++i) {
		if (groupList->at(i)->GetId() == selectedGroup->GetId()) {
			return i;
		}
	}
	
	return -1;
}

-(void)useEntertainmentArea:(int) position {
	auto bridge = self.huestream->GetActiveBridge();
	auto groupList = bridge->GetGroups();
	if (position >= 0 && position < groupList->size() && (!bridge->IsValidGroupSelected() || (bridge->IsValidGroupSelected() && groupList->at(position)->GetId() != bridge->GetGroup()->GetId()))) {
		self.huestream->SelectGroupAsync(groupList->at(position));
	}
}

-(void)resetHueData
{
	self.huestream->ResetAllPersistentDataAsync();
}

-(void)setLightsToGreen
{
	if (!self.huestream->IsBridgeStreaming())	{
		self.huestream->Start();
	}
	
	self.huestream->LockMixer();
	if (self.greenEffect != nil) {
		self.greenEffect->Finish();
	}
	if (self.leftEffect != nil)	{
		self.leftEffect->Finish();
	}
	if (self.rightEffect != nil) {
		self.rightEffect->Finish();
	}
	if (self.explosionEffect != nil) {
		self.explosionEffect->Finish();
	}

	self.greenEffect = std::make_shared<AreaEffect>("", 0);
	self.greenEffect->AddArea(Area::All);
	self.greenEffect->SetFixedColor(huestream::Color(0.0,1.0,0.0));
	self.greenEffect->Enable();
	self.huestream->AddEffect(self.greenEffect);
	self.huestream->UnlockMixer();
}

-(void)showAnimatedLightExample {
	if (!self.huestream->IsBridgeStreaming()) {
		self.huestream->Start();
	}
	
	self.huestream->LockMixer();
	
	if (self.greenEffect != nil) {
		self.greenEffect->Finish();
		
	}
	if (self.leftEffect != nil) {
		self.leftEffect->Finish();
	}
	if (self.rightEffect != nil) {
		self.rightEffect->Finish();
	}
	if (self.explosionEffect != nil) {
		self.explosionEffect->Finish();
	}

	// Create an animation which is fixed 0
	std::shared_ptr<ConstantAnimation> fixedZero = std::make_shared<ConstantAnimation>(0.0);

	// Create an animation which is fixed 1
	std::shared_ptr<ConstantAnimation> fixedOne = std::make_shared<ConstantAnimation>(1.0);

	// Create an animation which repeats a 2 second sawTooth 5 times
	PointListPtr pointList = std::make_shared<PointList>();
	//std::shared_ptr<std::vector<std::huestream::Point>> pointList = std::make_shared<std::vector<huestream::Point>>();
	pointList->push_back(std::make_shared<huestream::Point>(0, 0.0));
	pointList->push_back(std::make_shared<huestream::Point>(1000, 1.0));
	pointList->push_back(std::make_shared<huestream::Point>(2000, 0.0));
	std::shared_ptr<CurveAnimation> sawTooth = std::shared_ptr<CurveAnimation>(new CurveAnimation(5.0, pointList));

	// Create an effect on the left half of the room where blue is animated by sawTooth
	self.leftEffect = std::make_shared<AreaEffect>("LeftArea", 1);
	self.leftEffect->AddArea(Area::LeftHalf);
	self.leftEffect->SetColorAnimation(fixedZero, fixedZero, sawTooth);
	
	// Create a red virtual light source where x-axis position is animated by sawTooth
	self.rightEffect = std::make_shared<LightSourceEffect>("RightSource", 1);
	self.rightEffect->SetFixedColor(huestream::Color(1.0,0.0,0.0));
	self.rightEffect->SetPositionAnimation(sawTooth, fixedZero);
	self.rightEffect->SetRadiusAnimation(fixedOne);

	// Create effect from predefined explosionEffect
	self.explosionEffect = std::make_shared<ExplosionEffect>("explosion", 2);
	huestream::Color explosionColorRGB(1.0, 0.8, 0.4);
	Location explosionLocationXY(0, 1);
	double radius = 2.0;
	double duration_ms = 2000;
	double expAlpha_ms = 50;
	double expRadius_ms = 100;
	self.explosionEffect->PrepareEffect(explosionColorRGB, explosionLocationXY, duration_ms, radius, expAlpha_ms, expRadius_ms);

	// Now play all effects
	self.leftEffect->Enable();
	self.rightEffect->Enable();
	self.explosionEffect->Enable();
	self.huestream->AddEffect(self.leftEffect);
	self.huestream->AddEffect(self.rightEffect);
	self.huestream->AddEffect(self.explosionEffect);
	self.huestream->UnlockMixer();
}

-(void)newHueMessage:(HueStreamFeedbackMessage*) msg {
	dispatch_async(dispatch_get_main_queue(), ^(void) {
		self.hueMessageHandler(msg);
	});
}

@end


@implementation HueStreamFeedbackMessage
-(id) init {
	self = [super init];
	return self;
}

-(id) initWithFeedbackMessage: (FeedbackMessage) feedbackMessage {
	if (self = [super init]) {
		_feedbackId = (HueFeedbackId)feedbackMessage.GetId();
		_feedbackType = (HueFeedbackType)feedbackMessage.GetMessageType();
		
		if (_feedbackType == FEEDBACK_TYPE_USER) {
			CFStringRef msg = CFStringCreateWithCString(kCFAllocatorDefault, feedbackMessage.GetUserMessage().c_str(), kCFStringEncodingASCII);
			_userMessage = (__bridge NSString*)msg;
		}
	}
	
	return self;
}

@end
