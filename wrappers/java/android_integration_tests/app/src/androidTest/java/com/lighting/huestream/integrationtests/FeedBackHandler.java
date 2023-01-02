package com.lighting.huestream.integrationtests;

import com.lighting.huestream.FeedbackMessage;
import com.lighting.huestream.IFeedbackMessageHandler;

class FeedBackHandler extends IFeedbackMessageHandler {
    public void NewFeedbackMessage(FeedbackMessage message)
    {
        System.out.println(message.GetDebugMessage());
        if (message.GetMessageType() == FeedbackMessage.FeedbackType.FEEDBACK_TYPE_USER) {
            System.out.println(message.GetUserMessage());
        }
    }
}