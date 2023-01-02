//
// Created by nlv15377 on 3-5-2017.
//

#ifndef HUESDK_STUBTIMEPROVIDER_H
#define HUESDK_STUBTIMEPROVIDER_H

#include <huestream/common/time/TimeManager.h>

namespace huestream {
    class StubTimeProvider : public TimeManager {
     public:
        void AddMilliseconds(int msecs) {
            _now = _now + msecs;
        }
    };
}
#endif //HUESDK_STUBTIMEPROVIDER_H
