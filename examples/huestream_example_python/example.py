import socket
import HueStreamWrapper as hue

class HueStreamExample():
    class FeedbackHandler(hue.IFeedbackMessageHandler):
        def NewFeedbackMessage(self, message):
            print(message.GetDebugMessage())
            if message.GetMessageType() == hue.FeedbackMessage.FEEDBACK_TYPE_USER:
                print("\n*** " + message.GetUserMessage() + " ***\n")

    def __init__(self, app):
        config = hue.Config(app, socket.gethostname(), hue.PersistenceEncryptionKey("encryption_key"))
        self.huestream = hue.HueStream(config)
        self.handler = self.FeedbackHandler()
        self.huestream.RegisterFeedbackHandler(self.handler)
        
    def select_group(self):
        groups = self.huestream.GetLoadedBridgeGroups()
        index = -1
        while not (index >= 0 and index < len(groups)):
            for i, group in enumerate(groups):
                print("[" + str(i) + "] " + group.GetName())
            try:
                index = int(input("Enter [index]: "))
            except ValueError:
                pass
        self.huestream.SelectGroup(groups[index])
        
    def connect(self):
        self.huestream.ConnectBridge()

        while self.huestream.GetConnectionResult() != hue.Streaming:
            if self.huestream.GetLoadedBridgeStatus() == hue.BRIDGE_INVALID_GROUP_SELECTED:
                self.select_group()
            else:
                input("Press Enter to retry...")
                self.huestream.ConnectBridge()

    def animate_green_saw(self):
        effect = hue.AreaEffect("green_saw", 0)
        effect.AddArea(hue.Area().All)
        red = hue.ConstantAnimation(0.0)
        sawtooth = hue.PointVector()
        sawtooth.append(hue.Point(   0, 0.0))
        sawtooth.append(hue.Point(1000, 1.0))
        sawtooth.append(hue.Point(2000, 0.0))
        green = hue.CurveAnimation(hue.INF, sawtooth)
        blue = hue.ConstantAnimation(0.0)
        effect.SetColorAnimation(red, green, blue)
        
        self.huestream.LockMixer()
        self.huestream.AddEffect(effect)
        effect.Enable()
        self.huestream.UnlockMixer()
    
    def run(self):
        try:
            self.connect()
            self.animate_green_saw()
            input("Press Enter to quit...")
            self.huestream.ShutDown()
        except:
            self.huestream.ShutDown()
            raise

HueStreamExample("EdkPythonExample").run()
