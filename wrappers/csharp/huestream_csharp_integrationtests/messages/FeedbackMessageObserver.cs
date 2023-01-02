using System;
using System.Collections.Generic;
using System.Threading;
using huestream;
namespace huestream_tests
{
    public class FeedbackMessageObserver : FeedbackMessageHandler
    {
        public override void NewFeedbackMessage(FeedbackMessage message)
        {
            DisplayMessage(message);

            if (_isListening && message.GetId() == _idToListen)
            {
                _listeningEventHandle.Set();
            }
        }

        public void StartListening(FeedbackMessage.Id id)
        {
            if (_isListening)
                return;

            _idToListen = id;
            _isListening = true;
        }

        public void SetEventWaitHandle(EventWaitHandle handle)
        {
            _listeningEventHandle = handle;
        }

        private EventWaitHandle _listeningEventHandle;

        private bool _isListening = false;
        private FeedbackMessage.Id _idToListen;
    }
}
