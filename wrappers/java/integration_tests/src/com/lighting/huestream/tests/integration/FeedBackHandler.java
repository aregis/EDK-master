package com.lighting.huestream.tests.integration;

import com.lighting.huestream.FeedbackMessage;
import com.lighting.huestream.FeedbackMessage.FeedbackType;
import com.lighting.huestream.IFeedbackMessageHandler;

class FeedBackHandler extends IFeedbackMessageHandler {
    public void NewFeedbackMessage(FeedbackMessage message)
    {
        System.out.println(message.GetDebugMessage());
        if (message.GetMessageType() == FeedbackType.FEEDBACK_TYPE_USER) {
            System.out.println(message.GetUserMessage());
        }
    }
}
