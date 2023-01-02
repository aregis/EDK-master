/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_EFFECT_EFFECTS_BASE_EFFECT_H_
#define HUESTREAM_EFFECT_EFFECTS_BASE_EFFECT_H_

#include "huestream/common/serialize/Serializable.h"
#include "huestream/common/data/Color.h"
#include "huestream/common/data/Light.h"
#include "huestream/common/data/Group.h"
#include "huestream/common/time/ITimeProvider.h"

#include <memory>
#include <string>

namespace huestream {

    /**
     abstract base class for a light effect
     */
    class Effect : public Serializable {
    public:
        /**
         constructor
         @param name Name of the effect
         @param layer Layer the effect is on, effects with higher layer index are rendered on top of effects with lower layer index
         */
        explicit Effect(std::string name = "", unsigned int layer = 0);

        /**
         destructor
         */
        virtual ~Effect() {}

        /**
         effects may override this method to know about the current entertainment group used
         @note gets called at least when the effect is added to the engine and then only on group changes
         */
        virtual void UpdateGroup(GroupPtr group);

        /**
         effects override this method to possibly do some pre rendering before GetColor()
         @note gets called once per render cycle
         */
        virtual void Render() = 0;

        /**
         effects override this method to implement how they map lights to colors
         @note gets called once per light per render cycle
         @param light Referencee to light to get the current color from
         @return current color of the light
         */
        virtual Color GetColor(LightPtr light) = 0;

        /**
         set effect state to enabled, i.e. it is being rendered
         */
        virtual void Enable();

        /**
         set effect state to disabled, i.e. it is not being rendered
         */
        virtual void Disable();

        /**
         set effect state to finished, i.e. it is cleaned up from the engine
         */
        void Finish();

        /**
         check whether the effect is disabled
         @return whether the effect is disabled
         */
        virtual bool IsEnabled() const;

        /**
         check whether the effect is disabled
         @return whether the effect is disabled
         */
        bool IsDisabled() const;

        /**
         check whether the effect is finished
         @return whether the effect is finished
         */
        bool IsFinished() const;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

    /**
     set name of the effect, used for serialization or lookup by name
     @param name New name for the effect
     */
    PROP_DEFINE(Effect, std::string, name, Name);

    /**
     set layer the effect is on, effects with higher layer index are rendered on top of effects with lower layer index
     @param layer New layer the effect will be on
     */
    PROP_DEFINE(Effect, unsigned int, layer, Layer);

    protected:
        enum class State {
            Finished, Enabled, Disabled
        };
        State _state;
    };

    /**
     shared pointer to an huestream::Effect object
     */
    SMART_POINTER_TYPES_FOR(Effect)

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_EFFECTS_BASE_EFFECT_H_
