using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Reflection;
using System.Runtime.CompilerServices;
using huestream;
using HueEffectEditor.Annotations;
using Type = System.Type;

namespace HueEffectEditor
{
    public class PropertyData
    {
        public object Value;
        public PropertyInfo Info;
    }

    public class PropertyGridManager : INotifyPropertyChanged
    {
        private readonly Stack<PropertyData> _stack;

        public Type SelectedItemType
        {
            get { return _selectedItemType; }
            set
            {
                _selectedItemType = value;
                OnPropertyChanged(nameof(SelectedItemType));
            }
        }

        private List<Type> _listTypes;
        public List<Type> ListTypes
        {
            get { return _listTypes; }
            set
            {
                _listTypes = value;
                OnPropertyChanged(nameof(ListTypes));
            }
        }
        private bool _isBackEnabled;
        public bool IsBackEnabled
        {
            get { return _isBackEnabled; }
            set
            {
                _isBackEnabled = value;
                OnPropertyChanged(nameof(IsBackEnabled));
            }
        }

        private bool _isNewListItemEnabled;
        public bool IsNewListItemEnabled
        {
            get { return _isNewListItemEnabled; }
            set
            {
                _isNewListItemEnabled = value;
                OnPropertyChanged(nameof(IsNewListItemEnabled));
            }
        }

        private bool _isDeleteListItemEnabled;
        public bool IsDeleteListItemEnabled
        {
            get { return _isDeleteListItemEnabled; }
            set
            {
                _isDeleteListItemEnabled = value;
                OnPropertyChanged(nameof(IsDeleteListItemEnabled));
            }
        }

        private object _activeObject;
        private Type _selectedItemType;

        public object ActiveObject
        {
            get
            {
                return _activeObject;
            }
            set
            {
                _stack.Clear();
                _activeObject = value;
                var prop = new PropertyData();
                prop.Value = _activeObject;
                PushActiveItem(prop);
            }
        }


        private void PushActiveItem(PropertyData value)
        {
            _stack.Push(value);
            _activeObject = value.Value;
            OnPropertyChanged(nameof(ActiveObject));
            UpdateUI();

        }

        private void UpdateUI()
        {
            UpdateUIBackEnabled();
            UpdateNewListItemEnabled();
            UpdateNewListTypesAndSelectedItemType();

        }

        private void UpdateNewListTypesAndSelectedItemType()
        {
            if (!IsNewListItemEnabled)
                return;

            var derivedItems = GetDerivedTypesOfListItems();

            if (!IsNewListTypesList(derivedItems))
                return;
            ListTypes = derivedItems;
            UpdateSelectedItemType();
        }

        private void UpdateSelectedItemType()
        {
            if (ListTypes.Count != 0)
            {
                SelectedItemType = ListTypes[0];
            }
        }

        private List<Type> GetDerivedTypesOfListItems()
        {
            var list = ((WrappedList)ActiveObject).GetList();
            var derivedItems = ListReflection.GetDerivedItems(list);
            return derivedItems;
        }

        private void UpdateNewListItemEnabled()
        {
            IsNewListItemEnabled = ActiveObject is WrappedList;
        }

        private void UpdateUIBackEnabled()
        {
            IsBackEnabled = _stack.Count > 1;
        }

        private bool IsNewListTypesList(List<Type> newListTypes)
        {
            var isNewList = newListTypes.Count != ListTypes.Count;
            for (var i = 0; i < ListTypes.Count && !isNewList; i++)
            {
                isNewList = (newListTypes[i] != ListTypes[i]);
            }
            return isNewList;
        }

        private void PopActiveItem()
        {
            _stack.Pop();
            _activeObject = _stack.Peek().Value;
            OnPropertyChanged(nameof(ActiveObject));
            UpdateUI();
        }

        public PropertyGridManager()
        {
            _stack = new Stack<PropertyData>();
            _listTypes = new List<Type>();
        }

        public void HighProperty(string name)
        {
            if (_activeObject is WrappedList)
                return;

            var prop = GetPropFromActiveObjectByName(name);
            if (prop == null || IsDirectlyEditableValue(prop) || IsListValue(prop))
            {
                IsNewListItemEnabled = false;
                ListTypes = new List<Type>();
                return;
            }

            IsNewListItemEnabled = true;
            ListTypes = ListReflection.FindAllDerivedTypes(prop.Info.PropertyType);
        }

        public void SelectProperty(string name)
        {
            var prop = GetPropFromActiveObjectByName(name);
            if (prop == null || IsDirectlyEditableValue(prop))
                return;

            if (prop.Value == null)
                return;

            if (IsListValue(prop))
            {
                WrapList(prop);
                return;
            }

            PushActiveItem(prop);
        }

        private static bool IsListValue(PropertyData prop)
        {
            return prop.Value is IEnumerable;
        }

        private static bool IsDirectlyEditableValue(PropertyData prop)
        {
            return (prop.Info.PropertyType == typeof(string)) || prop.Info.PropertyType.IsPrimitive;
        }

        private PropertyData GetPropFromActiveObjectByName(string name)
        {
            var prop = new PropertyData();
            prop.Info = ActiveObject.GetType().GetProperty(name, BindingFlags.Instance | BindingFlags.Public);
            if (prop.Info == null)
                return null;

            prop.Value = prop.Info.GetMethod.Invoke(ActiveObject, null);
            return prop;
        }

        private void WrapList(PropertyData prop)
        {
            prop.Value = new WrappedListBuilder().Create(prop.Value);
            PushActiveItem(prop);
        }

        public void Back()
        {
            PopActiveItem();
        }

        public event PropertyChangedEventHandler PropertyChanged;

        [NotifyPropertyChangedInvocator]
        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public void NewItem(string name)
        {
            if (ActiveObject is WrappedList)
            {
                var wrappedList = ActiveObject as WrappedList;
                var list = wrappedList.GetList();
                ListReflection.AddNewDefaultItem(list, SelectedItemType);

                var prop = _stack.Pop();
                prop.Value = list;
                WrapList(prop);
            }
            else
            {
                var prop = GetPropFromActiveObjectByName(name);
                if (prop == null || IsDirectlyEditableValue(prop))
                    return;
                var newItem = Activator.CreateInstance(SelectedItemType);
                prop.Info.GetSetMethod().Invoke(_activeObject, new[] {newItem});

                var x = _stack.Peek();
                PopActiveItem();
                PushActiveItem(x);
            }

        }
    }
}