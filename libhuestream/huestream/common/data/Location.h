/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_COMMON_DATA_LOCATION_H_
#define HUESTREAM_COMMON_DATA_LOCATION_H_

#include "huestream/common/serialize/SerializerHelper.h"
#include "huestream/common/serialize/Serializable.h"

#include <string>

namespace huestream {

    /**
     2 dimensional location where (0,0) = center, negative x = left, positive x = right, negative y = back, positive y = front
     */
    class Location : public Serializable {
    public:
        static constexpr const char* type = "huestream.Location";

    /**
     set x coordinate
     */
    PROP_DEFINE(Location, double, x, X);

    /**
     set y coordinate
     */
    PROP_DEFINE(Location, double, y, Y);

    /**
    set z coordinate
    */
    PROP_DEFINE(Location, double, z, Z);

    public:
        Location();

        Location(double x, double y);

        /**
         constructor
         @param x X coordinate
         @param y Y coordinate
         @param z Z coordinate
         */
        Location(double x, double y, double z);

        /**
         destructor
         */
        virtual ~Location();

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        std::string GetTypeName() const override;
    };

    /**
     shared pointer to a huestream::Location object
     */
    SMART_POINTER_TYPES_FOR(Location)
}  // namespace huestream

#endif  // HUESTREAM_COMMON_DATA_LOCATION_H_
