/*#============================================
  #  ______  _____   _  __
  # |  ____||  __ \ | |/ /
  # | |__   | |  | || ' /
  # |  __|  | |  | ||  <
  # | |____ | |__| || . \
  # |______||_____/ |_|\_\  C# & Java wrapper
  #                                 Philips HUE
  #============================================*/

%include <stdint.i>
%include <std_string.i>
%include <std_vector.i>
%include <attribute.i>

%module(directors="1") HueStreamWrapper
%{
#ifdef ANDROID
#  include "support/jni/SDKSupportJNI.h"
#  include "support/jni/core/JNIEnvFactory.h"
#endif
%}
%{

#ifdef ANDROID
using huesdk_jni_core::JNIEnvFactory;

JavaVM* JNIEnvFactory::JVM = nullptr;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
  support::jni::init(vm);
  JNIEnvFactory::JVM = vm;
  
  JNIEnvFactory::Create();
  return JNIEnvFactory::JNIVersion;
}

void JNI_OnUnload(JavaVM* vm, void* reserved) {
  JNIEnvFactory::JVM = nullptr;
}
#endif
%}

#if defined(SWIGCSHARP)
  %include "csharp/TypeMap.i"
#endif



#if defined(SWIGCSHARP)
  %include "csharp/std_shared_ptr_upcast.i" // Customized shared_ptr for upcasting support for Serializable classes
  //%include <std_shared_ptr.i> // Fallback standard shared_ptr if you have trouble with upcasting
#elif defined(SWIGJAVA)
  %include <java/std_shared_ptr_patched.i> // Customized shared_ptr for fixing shared_ptr derived delegators
#else
  %include <std_shared_ptr.i>
#endif



//----------------------------------------------------
// SHARED pointer definitions
//----------------------------------------------------
%shared_ptr(huestream::Serializable)
%shared_ptr(huestream::Color)
%shared_ptr(std::vector<std::shared_ptr<huestream::Color>>)
%shared_ptr(huestream::Location)
%shared_ptr(std::vector<std::shared_ptr<huestream::Location>>)
%shared_ptr(huestream::IArea)
%shared_ptr(std::vector<std::shared_ptr<huestream::IArea>>)
%shared_ptr(huestream::Area)
%shared_ptr(huestream::CuboidArea)
%shared_ptr(huestream::Light)
%shared_ptr(std::vector<std::shared_ptr<huestream::Light>>)
%shared_ptr(huestream::Scene)
%shared_ptr(std::vector<std::shared_ptr<huestream::Scene>>)
%shared_ptr(huestream::Group)
%shared_ptr(std::vector<std::shared_ptr<huestream::Group>>)
%shared_ptr(huestream::Bridge)
%shared_ptr(std::vector<std::shared_ptr<huestream::Bridge>>)
%shared_ptr(huestream::HueStreamData)
%shared_ptr(huestream::IMessageTranslator)
%shared_ptr(huestream::DummyTranslator)
%shared_ptr(huestream::IStorageAccessor)
%shared_ptr(huestream::IBridgeStorageAccessor)
%shared_ptr(huestream::BridgeFileStorageAccessor)
%shared_ptr(huestream::IBasicGroupLightController)
%shared_ptr(huestream::BasicGroupLightController)
%shared_ptr(huestream::IConnectionFlow)
%shared_ptr(huestream::ConnectionFlow)
%shared_ptr(huestream::IConnect)
%shared_ptr(huestream::Connect)
%shared_ptr(huestream::HttpRequest)
%shared_ptr(huestream::IHttp)
%shared_ptr(huestream::Http)
%shared_ptr(huestream::IMessageDispatcher)
%shared_ptr(huestream::MessageDispatcher)
%shared_ptr(huestream::BridgeSettings)
%shared_ptr(huestream::StreamSettings)
%shared_ptr(huestream::AppSettings)
%shared_ptr(huestream::Config)
%shared_ptr(huestream::ObjectBuilderBase)
%shared_ptr(huestream::IStream)
%shared_ptr(huestream::Stream)
%shared_ptr(huestream::ITimeManager)
%shared_ptr(huestream::ObjectBuilder)
%shared_ptr(huestream::FileStorageAccessor)
%shared_ptr(huestream::ITimeProvider)
%shared_ptr(huestream::ITimeManager)
%shared_ptr(huestream::TimeManager)
%shared_ptr(huestream::IMixer)
%shared_ptr(huestream::Mixer)
%shared_ptr(huestream::IFeedbackMessageHandler)
%shared_ptr(huestream::Point)
%shared_ptr(std::vector<std::shared_ptr<huestream::Point>>)
%shared_ptr(huestream::CurveOptions)
%shared_ptr(huestream::CurveData)
%shared_ptr(huestream::Animation)
%shared_ptr(std::vector<std::shared_ptr<huestream::Animation>>)
%shared_ptr(huestream::Repeatable)
%shared_ptr(huestream::ConstantAnimation)
%shared_ptr(huestream::CurveAnimation)
%shared_ptr(huestream::RandomAnimation)
%shared_ptr(huestream::RepeatableAnimation)
%shared_ptr(huestream::TriggerableAnimation)
%shared_ptr(huestream::SequenceAnimation)
%shared_ptr(huestream::TweenAnimation)
%shared_ptr(huestream::FramesAnimation)
%shared_ptr(huestream::Effect)
%shared_ptr(huestream::AnimationEffect)
%shared_ptr(std::vector<std::shared_ptr<huestream::AnimationEffect>>)
%shared_ptr(huestream::ColorAnimationEffect)
%shared_ptr(huestream::LightSourceEffect)
%shared_ptr(huestream::SphereLightSourceEffect)
%shared_ptr(huestream::RadialEffect)
%shared_ptr(huestream::AreaEffect)
%shared_ptr(huestream::LightIteratorEffect)
%shared_ptr(huestream::ManualEffect)
%shared_ptr(huestream::ExplosionEffect)
%shared_ptr(huestream::HitEffect)
%shared_ptr(huestream::SequenceEffect)
%shared_ptr(huestream::MultiChannelEffect)
%shared_ptr(huestream::ITimeline)
%shared_ptr(huestream::Timeline)
%shared_ptr(huestream::LightScript)
%shared_ptr(huestream::IPlayer)
%shared_ptr(huestream::Channel)
%shared_ptr(std::vector<std::shared_ptr<huestream::Channel>>)
%shared_ptr(huestream::Action)
%shared_ptr(std::vector<std::shared_ptr<huestream::Action>>)
%shared_ptr(huestream::IConnector)
%shared_ptr(huestream::UdpConnector)
%shared_ptr(huestream::IHueStream)

%shared_ptr(huestream::HueStream)

//------------------------------------------------------
//Director support (inheritance in C#/Java)
//(With some fixes for shared_ptr support with director)
//------------------------------------------------------
#if defined(SWIGCSHARP)
  //IFeedbackMessageHandler supports C# inheritance
  %typemap(csdirectorin) std::shared_ptr<huestream::IFeedbackMessageHandler> "new IFeedbackMessageHandler($iminput, true)"
  %feature("director") huestream::IFeedbackMessageHandler;
  
  //Effect supports C# inheritance
  %typemap(csdirectorin) std::shared_ptr<huestream::Effect> "new Effect($iminput, true)"
  %typemap(csdirectorin) std::shared_ptr<huestream::Light> "new Light($iminput, true)"
  %typemap(csdirectorin) std::shared_ptr<huestream::Group> "new Group($iminput, true)"
  %feature("director") huestream::Effect;
  
  //LightSourceEffect supports C# inheritance
  //%typemap(csdirectorin) std::shared_ptr<huestream::LightSourceEffect> "new LightSourceEffect($iminput, true)"
  //%typemap(csdirectorout) std::shared_ptr<huestream::AnimationList> "AnimationVector.getCPtr($cscall).Handle"
  //%feature("director") huestream::LightSourceEffect;
  
#elif defined(SWIGJAVA)
  //IFeedbackMessageHandler supports Java inheritance
  %feature("director") huestream::IFeedbackMessageHandler;
  
  //Effect supports Java inheritance
  
//  %feature("director") huestream::Effect;
//  
//  %typemap(javadirectorin) std::shared_ptr<huestream::Light> "new $typemap(jstype, huestream::Light)($1,false)";
//  %typemap(directorin,descriptor="L$packagepath/$typemap(jstype, huestream::Light);") std::shared_ptr<huestream::Light> %{
//    *($&1_type*)&j$1 = &$1;
//  %}
//  
//  %typemap(javadirectorout) std::shared_ptr<huestream::Light> "$typemap(jstype, huestream::Light).getCPtr($javacall)";
//  %typemap(directorout) std::shared_ptr<huestream::Light> %{
//    $&1_type tmp = NULL;
//    *($&1_type*)&tmp = *($&1_type*)&$input;
//    if (!tmp) {
//      SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null $1_type");
//      return NULL;
//    }
//    $result = *tmp;
//  %}

#else
    %feature("director") huestream::IFeedbackMessageHandler;

#endif

//----------------------------------------------------
// C++ wrapper header includes
//----------------------------------------------------
%{
/* Includes the header in the wrapper code */
#include <huestream/common/serialize/ObjectBuilderBase.h>
#include <huestream/common/time/ITimeManager.h>
#include <huestream/common/serialize/SerializerHelper.h>
#include <huestream/common/serialize/Serializable.h>
#include <huestream/common/time/ITimeProvider.h>
#include <huestream/common/time/ITimeManager.h>
#include <huestream/common/time/TimeManager.h>
#include <huestream/common/language/IMessageTranslator.h>
#include <huestream/common/language/DummyTranslator.h>
#include <huestream/common/data/Location.h>
#include <huestream/common/data/Color.h>
#include <huestream/common/data/Light.h>
#include <huestream/common/data/Scene.h>
#include <huestream/common/data/Group.h>
#include <huestream/common/data/IArea.h>
#include <huestream/common/data/Area.h>
#include <huestream/common/data/CuboidArea.h>
#include <huestream/common/data/BridgeSettings.h>
#include <huestream/common/data/Bridge.h>
#include <huestream/common/data/HueStreamData.h>
#include <huestream/common/http/IBridgeHttpClient.h>
#include <huestream/common/http/BridgeHttpClient.h>
#include <huestream/common/storage/FileStorageAccessor.h>
#include <huestream/connect/FeedbackMessage.h>
#include <huestream/connect/IBasicGroupLightController.h>
#include <huestream/connect/BasicGroupLightController.h>
#include <huestream/connect/IConnectionFlow.h>
#include <huestream/connect/ConnectionFlow.h>
#include <huestream/connect/IFeedbackMessageHandler.h>
#include <huestream/connect/IBridgeStorageAccessor.h>
#include <huestream/connect/OperationResult.h>
#include <huestream/connect/IMessageDispatcher.h>
#include <huestream/connect/BridgeFileStorageAccessor.h>
#include <huestream/connect/MessageDispatcher.h>
#include <huestream/connect/OperationResult.h>
#include <huestream/connect/Connect.h>
#include <huestream/effect/IMixer.h>
#include <huestream/effect/Mixer.h>
#include <huestream/effect/lightscript/ITimeline.h>
#include <huestream/effect/lightscript/Timeline.h>
#include <huestream/effect/lightscript/LightScript.h>
#include <huestream/effect/animation/IPlayer.h>
#include <huestream/effect/animation/Player.h>
#include <huestream/effect/animation/ActionPlayer.h>
#include <huestream/effect/animation/data/Point.h>
#include <huestream/effect/animation/data/Vector.h>
#include <huestream/effect/animation/data/CurveData.h>
#include <huestream/effect/animation/data/CurveOptions.h>
#include <huestream/effect/animation/data/TweenType.h>
#include <huestream/effect/animation/animations/base/Animation.h>
#include <huestream/effect/animation/animations/base/RepeatableAnimation.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/animation/animations/CurveAnimation.h>
#include <huestream/effect/animation/animations/RandomAnimation.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream/effect/animation/animations/FramesAnimation.h>
#include <huestream/effect/animation/data/Channel.h>
#include <huestream/effect/effects/base/Effect.h>
#include <huestream/effect/effects/base/AnimationEffect.h>
#include <huestream/effect/lightscript/Action.h>
#include <huestream/effect/effects/base/ColorAnimationEffect.h>
#include <huestream/effect/effects/base/RadialEffect.h>
#include <huestream/effect/effects/LightSourceEffect.h>
#include <huestream/effect/effects/SphereLightSourceEffect.h>
#include <huestream/effect/effects/MultiChannelEffect.h>
#include <huestream/effect/effects/AreaEffect.h>
#include <huestream/effect/effects/LightIteratorEffect.h>
#include <huestream/effect/effects/ManualEffect.h>
#include <huestream/effect/effects/ExplosionEffect.h>
#include <huestream/effect/effects/HitEffect.h>
#include <huestream/effect/effects/SequenceEffect.h>
#include <huestream/stream/ProtocolSerializer.h>
#include <huestream/stream/StreamSettings.h>
#include <huestream/stream/IConnector.h>
#include <huestream/stream/UdpConnector.h>
#include <huestream/stream/IStream.h>
#include <huestream/stream/Stream.h>
#include <huestream/config/AppSettings.h>
#include <huestream/config/Config.h>
#include <huestream/config/ObjectBuilder.h>
#include <huestream/HueStream.h>

using namespace huestream;
%}

//----------------------------------------------------
// Vector support
//----------------------------------------------------
%template(PointVector) std::vector<shared_ptr<huestream::Point>>;
%template(LightVector) std::vector<shared_ptr<huestream::Light>>;
%template(SceneVector) std::vector<shared_ptr<huestream::Scene>>;
%template(GroupVector) std::vector<shared_ptr<huestream::Group>>;
%template(BridgeVector) std::vector<shared_ptr<huestream::Bridge>>;
%template(AreaVector) std::vector<shared_ptr<huestream::IArea>>;
%template(ColorVector) std::vector<shared_ptr<huestream::Color>>;
%template(LocationVector) std::vector<shared_ptr<huestream::Location>>;
%template(ChannelVector) std::vector<shared_ptr<huestream::Channel>>;
%template(EffectVector) std::vector<shared_ptr<huestream::Effect>>; 
%template(ActionVector) std::vector<shared_ptr<huestream::Action>>;
%template(AnimationEffectVector) std::vector<shared_ptr<huestream::AnimationEffect>>;
%template(StringVector) std::vector<std::string>;

//SWIG_STD_VECTOR_ENHANCED(shared_ptr<huestream::Animation>) //Enable this for IList support
%template(AnimationVector) std::vector<shared_ptr<huestream::Animation>>;

//----------------------------------------------------
// Wrappers for huestream/common/*
//----------------------------------------------------
%include <huestream/common/serialize/ObjectBuilderBase.h>
%include <huestream/common/serialize/SerializerHelper.h>
%include <huestream/common/serialize/Serializable.h>
%include <huestream/common/time/ITimeProvider.h>
%include <huestream/common/time/ITimeManager.h>
%include <huestream/common/time/TimeManager.h>
%include <huestream/common/language/IMessageTranslator.h>
%include <huestream/common/language/DummyTranslator.h>

%include <huestream/common/data/Location.h>
%attribute(huestream::Location, double, X, GetX, SetX);
%attribute(huestream::Location, double, Y, GetY, SetY);
%attribute(huestream::Location, double, Z, GetZ, SetZ);

%ignore huestream::Colors; 
%include <huestream/common/data/Color.h>
%attribute(huestream::Color, double, R, GetR, SetR);
%attribute(huestream::Color, double, G, GetG, SetG);
%attribute(huestream::Color, double, B, GetB, SetB);
%attribute(huestream::Color, double, Alpha, GetAlpha, SetAlpha);

%include <huestream/common/data/Light.h>
%attribute(huestream::Light, string, Id, GetId, SetId);
%attribute(huestream::Light, huestream::Color, Color, GetColor, SetColor);
%attribute(huestream::Light, huestream::Location, Position, GetPosition, SetPosition);

%include <huestream/common/data/Scene.h>
%attribute(huestream::Scene, string, Tag, GetTag, SetTag);
%attribute(huestream::Scene, string, Name, GetName, SetName);
%attribute(huestream::Scene, string, AppData, GetAppData, SetAppData);

%include <huestream/common/data/Group.h>
%attribute(huestream::Group, string, Id, GetId, SetId);
%attribute(huestream::Group, string, Name, GetName, SetName);
%attribute(huestream::Group, huestream::LightListPtr, Lights, GetLights, SetLights);

%include <huestream/common/data/IArea.h>
%include <huestream/common/data/Area.h>
%attribute(huestream::Area, huestream::Location, TopLeft, GetTopLeft, SetTopLeft);
%attribute(huestream::Area, huestream::Location, BottomRight, GetBottomRight, SetBottomRight);
%attribute(huestream::Area, string, Name, GetName, SetName);
%attribute(huestream::Area, bool, IsInverted, Inverted, SetInverted);
%include <huestream/common/data/CuboidArea.h>
%attribute(huestream::CuboidArea, huestream::Location, TopFrontLeft, GetTopFrontLeft, SetTopFrontLeft);
%attribute(huestream::CuboidArea, huestream::Location, BottomBackRight, GetBottomBackRight, SetBottomBackRight);
%attribute(huestream::CuboidArea, string, Name, GetName, SetName);
%attribute(huestream::CuboidArea, bool, IsInverted, Inverted, SetInverted);

%include <huestream/common/data/BridgeSettings.h>
%include <huestream/common/data/Bridge.h>
%attribute(huestream::Bridge, string, Name, GetName, SetName);
%attribute(huestream::Bridge, string, ModelId, GetModelId, SetModelId);
%attribute(huestream::Bridge, string, Apiversion, GetApiversion, SetApiversion);
%attribute(huestream::Bridge, string, Id, GetId, SetId);
%attribute(huestream::Bridge, string, IpAddress, GetIpAddress, SetIpAddress); 
%attribute(huestream::Bridge, bool, ValidIp, IsValidIp, SetIsValidIp);
%attribute(huestream::Bridge, bool, Authorized, IsAuthorized, SetIsAuthorized);
%attribute(huestream::Bridge, string, ClientKey, GetClientKey, SetClientKey);
%attribute(huestream::Bridge, string, User, GetUser, SetUser);
%attribute(huestream::Bridge, GroupListPtr, Groups, GetGroups, SetGroups);
%attribute(huestream::Bridge, string, SelectedGroup, GetSelectedGroup, SetSelectedGroup);

%include <huestream/common/data/HueStreamData.h>
%attribute(huestream::HueStreamData, BridgeListPtr, Bridges, GetBridges, SetBridges);

%include <huestream/common/http/IBridgeHttpClient.h>
%include <huestream/common/http/BridgeHttpClient.h>

%include <huestream/common/storage/IStorageAccessor.h>
%include <huestream/common/storage/FileStorageAccessor.h>

//----------------------------------------------------
// Wrappers for huestream/connect/*
//----------------------------------------------------
%include <huestream/connect/IBridgeSearcher.h>
%include <huestream/connect/IBridgeAuthenticator.h>
%include <huestream/connect/IBridgeStorageAccessor.h>
%include <huestream/connect/OperationResult.h>
%include <huestream/connect/BridgeFileStorageAccessor.h>
%include <huestream/connect/IConnectionFlowFactory.h>
%include <huestream/connect/IMessageDispatcher.h>
%include <huestream/connect/MessageDispatcher.h>
%include <huestream/connect/FeedbackMessage.h>
%include <huestream/connect/IBasicGroupLightController.h>
%include <huestream/connect/BasicGroupLightController.h>
%include <huestream/connect/IConnectionFlow.h>
%include <huestream/connect/ConnectionFlow.h>
%include <huestream/connect/IFeedbackMessageHandler.h>
%include <huestream/connect/Connect.h>

//----------------------------------------------------
// Wrappers for huestream/effect/*
//----------------------------------------------------
%include <huestream/effect/animation/data/Point.h>
  %attribute(huestream::Point, double, X, GetX, SetX);
  %attribute(huestream::Point, double, Y, GetY, SetY);

%include <huestream/effect/animation/data/Vector.h>
// TODO:: Add any attributes as well? Not needed now?

%include <huestream/effect/animation/data/CurveOptions.h>
// TODO:: Needs fixing ... Remove Nullable?
//%attribute(huestream::CurveOptions, Nullable<double>, ClipMin, GetClipMin, SetClipMin);
//%attribute(huestream::CurveOptions, Nullable<double>, ClipMax, GetClipMax, SetClipMax);
%attribute(huestream::CurveOptions, double, MultiplyFactor, GetMultiplyFactor, SetMultiplyFactor);

namespace huestream {
%template(NullableOptions) Nullable<CurveOptions>;
%template(NullableDouble) Nullable<double>;
}

%include <huestream/effect/animation/data/TweenType.h>
%include <huestream/effect/animation/data/CurveData.h>
  %attribute(huestream::CurveData, PointListPtr, Points, GetPoints, SetPoints);
  
%include <huestream/effect/animation/animations/base/Animation.h>
    %attribute(huestream::Animation, double, Value, GetValue, SetValue);

%include <huestream/effect/animation/animations/ConstantAnimation.h>

%include <huestream/effect/animation/animations/base/RepeatableAnimation.h>
    %attribute(huestream::RepeatableAnimation, double, RepeatTimes, GetRepeatTimes, SetRepeatTimes);

%include <huestream/effect/animation/animations/CurveAnimation.h>
    %attribute(huestream::CurveAnimation, CurveData, CurveData, GetCurveData, SetCurveData);

%include <huestream/effect/animation/animations/RandomAnimation.h>
    %attribute(huestream::RandomAnimation, double, MinValue, GetMinValue, SetMinValue);
    %attribute(huestream::RandomAnimation, double, MaxValue, GetMaxValue, SetMaxValue);
    %attribute(huestream::RandomAnimation, double, MinInterval, GetMinInterval, SetMinInterval);
    %attribute(huestream::RandomAnimation, double, MaxInterval, GetMaxInterval, SetMaxInterval);
    %attribute(huestream::RandomAnimation, TweenType, TweenType, GetTweenType, SetTweenType);
    %attribute(huestream::RandomAnimation, double, Length, GetLength, SetLength);

%include <huestream/effect/animation/animations/TweenAnimation.h>
    %attribute(huestream::TweenAnimation, double, Begin, GetBegin, SetBegin);
    %attribute(huestream::TweenAnimation, double, End, GetEnd, SetEnd);
    %attribute(huestream::TweenAnimation, double, Time, GetTime, SetTime);
    %attribute(huestream::TweenAnimation, bool, IsBeginValuePresent, BeginValuePresent, SetBeginValuePresent);
    %attribute(huestream::TweenAnimation, TweenType, TweenType, GetTweenType, SetTweenType);

%include <huestream/effect/animation/animations/FramesAnimation.h>

%include <huestream/effect/animation/animations/base/TriggerableAnimation.h>
%include <huestream/effect/animation/animations/SequenceAnimation.h>

    %attribute(huestream::SequenceAnimation, AnimationListPtr, Sequences, GetSequences, SetSequences);

%include <huestream/effect/animation/data/Channel.h>
    %attribute(huestream::Channel, huestream::Location, Position, GetPosition, SetPosition);
    %attribute(huestream::Channel, huestream::AnimationPtr, R, GetR, SetR);
    %attribute(huestream::Channel, huestream::AnimationPtr, G, GetG, SetG);
    %attribute(huestream::Channel, huestream::AnimationPtr, B, GetB, SetB);
    %attribute(huestream::Channel, huestream::AnimationPtr, A, GetA, SetA);

%include <huestream/effect/effects/base/Effect.h>

%include <huestream/effect/effects/base/AnimationEffect.h>
    %attribute(huestream::AnimationEffect, huestream::AnimationPtr, Speed, GetSpeed, SetSpeed);

%include <huestream/effect/lightscript/Action.h>
    %attribute(huestream::Action, uint64_t, StartPosition, GetStartPosition, SetStartPosition);
    %attribute(huestream::Action, uint64_t, EndPosition, GetEndPosition, SetEndPosition);
    %attribute(huestream::Action, huestream::AnimationEffectPtr, Effect, GetEffect, SetEffect);

%include <huestream/effect/effects/base/ColorAnimationEffect.h>
    %attribute(huestream::ColorAnimationEffect, huestream::AnimationPtr, R, GetR, SetR);
    %attribute(huestream::ColorAnimationEffect, huestream::AnimationPtr, G, GetG, SetG);
    %attribute(huestream::ColorAnimationEffect, huestream::AnimationPtr, B, GetB, SetB);
    %attribute(huestream::ColorAnimationEffect, huestream::AnimationPtr, A, GetA, SetA);
    %attribute(huestream::ColorAnimationEffect, huestream::AnimationPtr, I, GetI, SetI);
    %attribute(huestream::ColorAnimationEffect, bool, IsOpacityBoundToIntensity, OpacityBoundToIntensity, SetOpacityBoundToIntensity);

%include <huestream/effect/effects/LightSourceEffect.h>
    %attribute(huestream::LightSourceEffect, huestream::AnimationPtr, Radius, GetRadius, SetRadius);
    %attribute(huestream::LightSourceEffect, huestream::AnimationPtr, X, GetX, SetX);
    %attribute(huestream::LightSourceEffect, huestream::AnimationPtr, Y, GetY, SetY);

%include <huestream/effect/effects/SphereLightSourceEffect.h>
    %attribute(huestream::SphereLightSourceEffect, huestream::AnimationPtr, Radius, GetRadius, SetRadius);
    %attribute(huestream::SphereLightSourceEffect, huestream::AnimationPtr, X, GetX, SetX);
    %attribute(huestream::SphereLightSourceEffect, huestream::AnimationPtr, Y, GetY, SetY);

%include <huestream/effect/effects/base/RadialEffect.h>
    %attribute(huestream::RadialEffect, double, Angle, GetAngle, SetAngle);

%include <huestream/effect/effects/MultiChannelEffect.h>
    %attribute(huestream::MultiChannelEffect, huestream::ChannelListPtr, Channels, GetChannels, SetChannels);

%include <huestream/effect/effects/AreaEffect.h>
    %attribute(huestream::AreaEffect, huestream::AreaListPtr, Areas, GetAreas, SetAreas);

%include <huestream/effect/effects/LightIteratorEffect.h>  
    %attribute(huestream::LightIteratorEffect, huestream::IterationOrder, Order, GetOrder, SetOrder);
    %attribute(huestream::LightIteratorEffect, huestream::IterationMode, Mode, GetMode, SetMode);
    %attribute(huestream::LightIteratorEffect, double, Offset, GetOffset, SetOffset);
    %attribute(huestream::LightIteratorEffect, bool, IsInvertOrder, InvertOrder, SetInvertOrder);

%include <huestream/effect/effects/ManualEffect.h>
%include <huestream/effect/effects/ExplosionEffect.h>
%include <huestream/effect/effects/HitEffect.h>
%include <huestream/effect/effects/SequenceEffect.h>

%include <huestream/effect/lightscript/ITimeline.h>
%include <huestream/effect/lightscript/Timeline.h>

%include <huestream/effect/lightscript/LightScript.h>
    %attribute(huestream::LightScript, string, Name, GetName, SetName);
    %attribute(huestream::LightScript, LocationListPtr, IdealSetup, GetIdealSetup, SetIdealSetup);
    %attribute(huestream::LightScript, long, Length, GetLength, SetLength);
    %attribute(huestream::LightScript, ActionListPtr, Actions, GetActions, SetActions);

%include <huestream/effect/IMixer.h>
%include <huestream/effect/Mixer.h>

//----------------------------------------------------
// Wrappers for huestream/stream/*
//----------------------------------------------------
%include <huestream/stream/ProtocolSerializer.h>
%include <huestream/stream/StreamSettings.h>
%include <huestream/stream/IStream.h>
%include <huestream/stream/Stream.h>
%include <huestream/stream/IConnector.h>
%include <huestream/stream/UdpConnector.h>

//----------------------------------------------------
// Wrappers for huestream/config/*
//----------------------------------------------------
%include <huestream/config/AppSettings.h>
%include <huestream/config/ObjectBuilder.h>
%include <huestream/config/Config.h>

%include <huestream/IHueStream.h>
%include <huestream/HueStream.h>

