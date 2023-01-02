/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_IMESSAGEDISPATCHER_H_
#define HUESTREAM_CONNECT_IMESSAGEDISPATCHER_H_

#include <functional>
#include <memory>

using std::shared_ptr;

typedef std::function<void()> DispatchAction;

namespace huestream {

#define MESSAGE_DISPATCH(dispatcher, __action__) \
  dispatcher->Queue(__action__);

#define MESSAGE_DISPATCH_P0(dispatcher, method) \
  MESSAGE_DISPATCH(dispatcher, std::bind(&method, this));

#define MESSAGE_DISPATCH_P1(dispatcher, method, value) \
  MESSAGE_DISPATCH(dispatcher, std::bind(&method, this, value));

#define MESSAGE_DISPATCH_P2(dispatcher, method, value1, value2) \
  MESSAGE_DISPATCH(dispatcher, std::bind(&method, this, value1, value2));

#define MESSAGE_DISPATCH_P3(dispatcher, method, value1, value2, value3) \
  MESSAGE_DISPATCH(dispatcher, std::bind(&method, this, value1, value2, value3));

    class IMessageDispatcher {
    public:
        virtual ~IMessageDispatcher() = default;

        virtual void Queue(DispatchAction action) = 0;

        virtual void Execute(bool useThisTread = false) = 0;

        virtual void Stop() = 0;
    };

    typedef shared_ptr<IMessageDispatcher> MessageDispatcherPtr;

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_IMESSAGEDISPATCHER_H_
