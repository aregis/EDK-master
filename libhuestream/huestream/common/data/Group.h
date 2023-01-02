/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_COMMON_DATA_GROUP_H_
#define HUESTREAM_COMMON_DATA_GROUP_H_

#include "huestream/common/data/Light.h"
#include "huestream/common/data/Scene.h"

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <map>

namespace huestream {

    /**
     class of entertainment group, indicating by what reference it has been created
     */
    enum GroupClass {
        GROUPCLASS_TV,
        GROUPCLASS_FREE,
        GROUPCLASS_SCREEN,      ///< Channels are organized around content from a screen
        GROUPCLASS_MUSIC,       ///< Channels are organized for music synchronization
        GROUPCLASS_3DSPACE,     ///< Channels are organized to provide 3d spacial effects
        GROUPCLASS_OTHER        ///< General use case
    };

    /**
     node which is the proxy for sending stream messages in the mesh network
     */
    typedef struct {
        std::string uri;
        std::string mode;
        std::string name;
        std::string model;
        bool isReachable;
    } GroupProxyNode;

    typedef std::map<std::string, std::vector<std::string>> GroupChannelToPhysicalLightMap;
    typedef std::shared_ptr<std::map<std::string, std::vector<std::string>>> GroupChannelToPhysicalLightMapPtr;

    /**
     defintion of a group of lights used as an entertainment setup to play light effects on
     */
    class Group : public Serializable {
    public:
        static constexpr const char* type = "huestream.Group";

    /**
     Set identifier of this group
     @note mostly internal use, imported from bridge
     */
    PROP_DEFINE(Group, std::string, id, Id);

    /**
     Id representing the grouped light resource of this group
     @note mostly internal use, imported from bridge
    */
    PROP_DEFINE(Group, std::string, groupedLightId, GroupedLightId);

    /**
     Set user given name of this group
     @note mostly internal use, imported from bridge
     */
    PROP_DEFINE(Group, std::string, name, Name);

    /**
     Set class type of this group
     @note mostly internal use, imported from bridge
     */
    PROP_DEFINE(Group, GroupClass, classType, ClassType);

    /**
     Set list of light channels in this group
     @note mostly internal use, imported from bridge
     */
    PROP_DEFINE(Group, LightListPtr, lights, Lights);

    /**
     Set list of physical lights in this group
     @note mostly internal use, imported from bridge
    */
    PROP_DEFINE(Group, LightListPtr, physicalLights, PhysicalLights);

    /**
     Set whether this group is currently being streamed to
     @note mostly internal use, imported from bridge
     */
    PROP_DEFINE_BOOL(Group, bool, active, Active);

    /**
     Set id of application currently streaming to this group
     @note mostly internal use, imported from bridge
     */
    PROP_DEFINE(Group, std::string, owner, Owner);

    /**
     Set name of application currently streaming to this group
     @note mostly internal use, imported from bridge
     */
    PROP_DEFINE(Group, std::string, ownerName, OwnerName);

    /**
     Set proxy node
     @note mostly internal use, imported from bridge
     */
    PROP_DEFINE(Group, GroupProxyNode, proxyNode, ProxyNode);

    /**
     Get list of scenes for this group
     */
    PROP_DEFINE(Group, SceneListPtr, scenes, Scenes);

    /**
     Get on state for this group
     @note only relevant when not streaming
     */
    PROP_DEFINE_BOOL(Group, bool, onState, OnState);

    /**
     Get brightness state for this group
     @note only relevant when not streaming
     */
    PROP_DEFINE(Group, double, brightnessState, BrightnessState);

    /**
     Get channels - physical lights association map for this group
     */
    PROP_DEFINE(Group, GroupChannelToPhysicalLightMapPtr, channelToPhysicalLightsMap, ChannelToPhysicalLightsMap);

    public:
        /**
         constructor
         */
        Group();

        /**
         destructor
         */
        virtual ~Group();

        /**
         [deprecated] add a light to this group (z coordinate forced to 0)
         */
        void AddLight(std::string id, double x, double y, std::string name = "", std::string model = "", std::string archetype = "", bool reachable = true);

        /**
         add a light to this group
         */
        void AddLight(std::string id, double x, double y, double z, std::string name = "", std::string model = "", std::string archetype = "", bool reachable = true);

        void AddLight(Light& light);

        /**
         get readable version of the owner name
         */
        std::string GetFriendlyOwnerName() const;

        /**
         get owner application name
         */
        std::string GetOwnerApplicationName() const;

        /**
         get owner device name (if available, else empty string)
         */
        std::string GetOwnerDeviceName() const;

        LightList GetChannelPhysicalLights(LightPtr channel);

        virtual void Serialize(JSONNode *node) const override;

        virtual void Deserialize(JSONNode *node) override;

        virtual std::string GetTypeName() const override;

        virtual Group* Clone();

    private:
        double Clip(double value, double min, double max) const;
        void SerializeClass(JSONNode *node) const;
        void DeserializeClass(JSONNode *node);
        std::vector<std::string> Split(const std::string& s, char delimiter) const;

        static const std::map<GroupClass, std::string> _classSerializeMap;
    };

    /**
     shared pointer to a huestream::Group object
     */
    SMART_POINTER_TYPES_FOR(Group)
}  // namespace huestream

#endif  // HUESTREAM_COMMON_DATA_GROUP_H_
