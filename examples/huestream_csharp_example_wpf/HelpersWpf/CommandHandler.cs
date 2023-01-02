using System;
using System.Windows.Input;

namespace HueStreamExampleWPF.HelpersWpf
{
    public class CommandHandler : ICommand
    {
        private readonly Action _action;
        private bool _canExecute;
        private bool _enabled;

        public CommandHandler(Action action, bool canExecute)
        {
            _enabled = true;
            _action = action;
            _canExecute = canExecute;
        }

        public void SetCanExecute(bool val)
        {
            _canExecute = val;
        }

        public bool CanExecute(object parameter)
        {
            return _enabled && _canExecute;
        }

        public event EventHandler CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }

        public void Execute(object parameter)
        {
            _action();
        }

        public void SetEnabled(bool val)
        {
            _enabled = val;
        }
    }
}