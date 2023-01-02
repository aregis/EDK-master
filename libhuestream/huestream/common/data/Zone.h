/*******************************************************************************
Copyright (C) 2020 Signify Holding
All Rights Reserved.
********************************************************************************/
/** @file */

#ifndef HUESTREAM_COMMON_DATA_ZONE_H_
#define HUESTREAM_COMMON_DATA_ZONE_H_

#include "huestream/common/data/Group.h"

namespace huestream
{
	enum ZoneArchetype
	{
		ZONEARCHETYPE_COMPUTER,
		ZONEARCHETYPE_OTHER
	};

	/**
	definition of a group of lights used to set scenes on
	*/
	class Zone: public Group
	{
	public:
		static constexpr const char* type = "huestream.Zone";

		PROP_DEFINE(Zone, ZoneArchetype, archetype, Archetype);
		PROP_DEFINE(Zone, std::string, idV1, IdV1);

	public:
		/**
		constructor
		*/
		Zone();

		/**
		destructor
		*/
		virtual ~Zone();

		Zone* Clone() override;

		virtual void Serialize(JSONNode *node) const override;
		virtual void Deserialize(JSONNode *node) override;
		virtual std::string GetTypeName() const override;

		ScenePtr GetSceneById(const std::string& sceneId);

	private:
		static const std::map<ZoneArchetype, std::string> _archetypeSerializeMap;

		void SerializeArchetype(JSONNode *node) const;
		void DeserializeArchetype(JSONNode *node);
	};

	/**
	shared pointer to a huestream::Zone object
	*/
	SMART_POINTER_TYPES_FOR(Zone)
}  // namespace huestream

#endif  // HUESTREAM_COMMON_DATA_ZONE_H_
