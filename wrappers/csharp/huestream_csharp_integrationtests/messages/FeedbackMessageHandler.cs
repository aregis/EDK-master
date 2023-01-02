using System;
using huestream;
namespace huestream_tests
{
    public class FeedbackMessageHandler : IFeedbackMessageHandler
    {
        public override void NewFeedbackMessage(FeedbackMessage message)
        {
            DisplayMessage(message);
        }

        protected void DisplayMessage(FeedbackMessage message)
        {
            Console.WriteLine($"DEBUG: {message.GetDebugMessage()}");
            if (message.GetMessageType() == FeedbackMessage.FeedbackType.FEEDBACK_TYPE_USER)
            {
                Console.WriteLine($"USER: {message.GetUserMessage()}");
            }
        }
    }
}
