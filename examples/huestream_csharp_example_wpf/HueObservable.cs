using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Windows.Input;
using System.Windows.Threading;
using huestream;
using HueStreamExampleWPF.HelpersWpf;

namespace HueStreamExampleWPF
{
    class HueObservable : INotifyPropertyChanged
    {
        private Hue _hue;
        private Bridge _bridge;
        private List<CommandHandler> _commands;

        private CommandHandler _initializeCommand;
        public CommandHandler InitializeCommand => _initializeCommand ?? (_initializeCommand = new CommandHandler(Initialize, true));

        private CommandHandler _initializeSimulatorCommand;
        public CommandHandler InitializeSimulatorCommand => _initializeSimulatorCommand ?? (_initializeSimulatorCommand = new CommandHandler(InitializeSimulator, true));

        private CommandHandler _connectCurrentBridgeInfoCommand;
        public CommandHandler ConnectCurrentBridgeInfoCommand => _connectCurrentBridgeInfoCommand ?? (_connectCurrentBridgeInfoCommand = new CommandHandler(ConnectCurrentBridgeInfo, false));

        private CommandHandler _connectManualIpCommand;
        public CommandHandler ConnectManualIpCommand => _connectManualIpCommand ?? (_connectManualIpCommand = new CommandHandler(ConnectManualIp, false));

        private CommandHandler _effectExplosionCommand;
        public CommandHandler EffectExplosionCommand => _effectExplosionCommand ?? (_effectExplosionCommand = new CommandHandler(EffectExplosion, false));

        private CommandHandler _brightnessSawCommand;

        public CommandHandler BrightnessSawCommand => _brightnessSawCommand ?? (_brightnessSawCommand = new CommandHandler(EffectBrighnessSaw, false));


        public string ManualIp { get; set; }
        public ObservableCollection<string> ConnectionFlowMessages { get; set; }

        public string IpAddress
        {
            get { return _bridge.GetIpAddress(); }
            set
            {
                _bridge.SetIpAddress(value);
                RaisePropertyChangedEvent("IpAddress");
            }
        }

        public string User
        {
            get { return _bridge.GetUser(); }
            set
            {
                _bridge.SetUser(value);
                RaisePropertyChangedEvent("User");
            }
        }

       
        public event PropertyChangedEventHandler PropertyChanged;
        protected void RaisePropertyChangedEvent(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public HueObservable()
        {
            _bridge = new Bridge(new BridgeSettings());
            ConnectionFlowMessages = new ObservableCollection<string>();
            ManualIp = "";
        }

        public void Update(Bridge bridge)
        {
            _bridge = bridge;
            RaiseAll();
        }

        private void Initialize()
        {
            InitializeCommand.SetCanExecute(false);
            InitializeSimulatorCommand.SetCanExecute(false);
            ConnectCurrentBridgeInfoCommand.SetCanExecute(true);
            ConnectManualIpCommand.SetCanExecute(true);
            _hue.Initialize();
        }

        private void InitializeSimulator()
        {
            InitializeCommand.SetCanExecute(false);
            InitializeSimulatorCommand.SetCanExecute(false);
            _hue.InitializeSimulator();

            ManualIp = "127.0.0.1";
            ConnectManualIp();
        }

        private void ConnectCurrentBridgeInfo()
        {
            DisableAllCommands();
            _hue.ConnectCurrent();
        }

        private void EffectExplosion()
        {
            _hue.PlayExplosion();
        }

        private void EffectBrighnessSaw()
        {
            _hue.PlayBrightnessSawEffect();
        }

        private void EnableAllCommands()
        {
            foreach (var command in _commands)
            {
                command.SetEnabled(true);
            }
        }

        private void DisableAllCommands()
        {
            foreach (var command in _commands)
            {
                command.SetEnabled(false);
            }
        }

        private void ConnectManualIp()
        {
            DisableAllCommands();
            _hue.ConnectManualIp(ManualIp);
        }

        private void RaiseAll()
        {
            RaisePropertyChangedEvent("IpAddress");
            RaisePropertyChangedEvent("User");
            RaisePropertyChangedEvent("ModelId");
        }

        
        public void SetModel(Hue hue, Dispatcher dispatcher)
        {
            _commands = new List<CommandHandler> { InitializeCommand, InitializeSimulatorCommand, ConnectCurrentBridgeInfoCommand, ConnectManualIpCommand, EffectExplosionCommand };

            _hue = hue;
            _hue.ConnectMessage += (s, message) =>
            {
                var id = message.GetId();
                if (message.GetBridge() != null)
                    Update(message.GetBridge());

                
                var messageText = message.GetTag();
                dispatcher.BeginInvoke((System.Action)delegate
                {
                    if (id == FeedbackMessage.Id.ID_USERPROCEDURE_FINISHED)
                    {
                        EnableAllCommands();
                        if (_hue.IsBridgeStreaming())
                        {
                            EnableEffectCommands();
                        }
                    }

                    ConnectionFlowMessages.Add(messageText);
                    CommandManager.InvalidateRequerySuggested();
                });
            };
        }

        private void EnableEffectCommands()
        {
            EffectExplosionCommand.SetCanExecute(true);
            BrightnessSawCommand.SetCanExecute(true);
        }

        public void ShutDown()
        {
            _hue.ShutDown();
        }
    }
}