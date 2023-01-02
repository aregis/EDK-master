/*******************************************************************************
Copyright (C) 2020 Signify Holding
All Rights Reserved.
********************************************************************************/

#include <huestream/common/data/Zone.h>

namespace huestream
{
	const std::map<ZoneArchetype, std::string> Zone::_archetypeSerializeMap = {
		{ ZONEARCHETYPE_COMPUTER, "computer" },
		{ ZONEARCHETYPE_OTHER,    "other" }
	};

	PROP_IMPL(Zone, ZoneArchetype, archetype, Archetype);
	PROP_IMPL(Zone, std::string, idV1, IdV1);

	Zone::Zone() : Group(), _idV1(""), _archetype(ZONEARCHETYPE_OTHER) {
	}

	Zone::~Zone()	{
	}

	Zone* Zone::Clone() {
		auto z = new Zone(*this);
		z->SetLights(clone_list(_lights));
		z->SetScenes(clone_list(_scenes));
		return z;
	}

	void Zone::Serialize(JSONNode *node) const {
		Group::Serialize(node);

		SerializeArchetype(node);
		SerializeValue(node, AttributeIdV1, _idV1);
	}

	void Zone::Deserialize(JSONNode *node) {
		Group::Deserialize(node);

		DeserializeArchetype(node);
		DeserializeValue(node, AttributeIdV1, &_idV1, "");
	}

	void Zone::SerializeArchetype(JSONNode *node) const	{
		auto it = _archetypeSerializeMap.find(_archetype);
		if (it != _archetypeSerializeMap.end())
		{
			SerializeValue(node, AttributeArchetype, it->second);
		}
		else
		{
			SerializeValue(node, AttributeArchetype, "other");
		}
	}

	void Zone::DeserializeArchetype(JSONNode *node)	{
		std::string archetypeString;
		DeserializeValue(node, AttributeArchetype, &archetypeString, "other");

		_archetype = ZONEARCHETYPE_OTHER;

		for (auto it = _archetypeSerializeMap.begin(); it != _archetypeSerializeMap.end(); ++it)
		{
			if (it->second == archetypeString)
			{
				_archetype = it->first;
				break;
			}
		}
	}

	std::string Zone::GetTypeName() const {
		return type;
	}

	ScenePtr Zone::GetSceneById(const std::string& sceneId) {
		if (_scenes == nullptr) {
			return nullptr;
		}

		for (auto sceneIt = _scenes->begin(); sceneIt != _scenes->end(); ++sceneIt) {
			if ((*sceneIt)->GetId() == sceneId) {
				return *sceneIt;
			}
		}

		return nullptr;
	}
}
