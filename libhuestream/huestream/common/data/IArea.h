/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_DATA_IAREA_H_
#define HUESTREAM_COMMON_DATA_IAREA_H_

#include "huestream/common/serialize/Serializable.h"
#include "huestream/common/data/Location.h"

#include <memory>

namespace huestream {
    
    class IArea : public Serializable {
    public:
        virtual ~IArea() = default;

        /**
         create an exact clone of this area as a new object
         */
        virtual std::shared_ptr<IArea> Clone() const = 0;

        /**
         check if a location is within this area
         */
        virtual bool isInArea(const Location &location) const = 0;
    };
    
    /**
     shared pointer to a huestream::IArea object
     */
    typedef std::shared_ptr<IArea> AreaPtr;
    typedef std::vector<AreaPtr> AreaList;
    typedef std::shared_ptr<AreaList> AreaListPtr;
    
}  // namespace huestream

#endif  // HUESTREAM_COMMON_DATA_IAREA_H_
