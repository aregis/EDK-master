using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NUnit.Framework.Internal;
using NUnit.Framework;

namespace HueEffectEditor
{
    class MySimpleManagerClass
    {
        public MySimpleDataClass Data { get; set; }
        public List<MyListItem> Items { get; set; }
    }

    class MySimpleDataClass
    {
        public string Hi { get; set; }    
    }

    abstract class MyListItem
    {
        public int Cxt { get; set; }
    }

    class MyListItemA : MyListItem
    {
        public int Cat { get; set; }
    }

    class MyListItemB : MyListItem
    {
        public int Cbt { get; set; }
    }

    [TestFixture]
    public class TestPropertyGridManager
    {
        private PropertyGridManager _manager;
        private List<PropertyChangedEventArgs> _calls;

        [SetUp]
        public void SetUp()
        {
            _calls = new List<PropertyChangedEventArgs>();
            _manager = new PropertyGridManager();
            _manager.PropertyChanged += (sender, args) => { _calls.Add(args); };
        }

        [Test]
        public void TestSetActiveObjectRaisesNotifiyChangedEvents()
        {
            SetActiveObject(new MySimpleManagerClass());
        }

        [Test]
        public void TestSelectStringProperty_IsIgnored()
        {
            SetActiveObject(new MySimpleDataClass());

            _manager.SelectProperty("Hi");
            AssertNoNotifyChangedHappendAnymore();
        }

        [Test]
        public void TestSelectClassProperty_RaisesNotifyEvents_ActiveObject_And_IsBackActive()
        {
            SetActiveObject(new MySimpleManagerClass { Data = new MySimpleDataClass() });

            _manager.SelectProperty("Data");
            AssertNotifiyChangedActiveObjectHappend();
            AssertNotifiyIsBackEnabledHappend(true);
            AssertNotifiyIsNewItemEnabled(true);
            AssertNoNotifyChangedHappendAnymore();

            Assert.IsInstanceOf<MySimpleDataClass>(_manager.ActiveObject);

        }

        [Test]
        public void TestSelectClassProperty_ThatIsNull_IsIgnored()
        {
            SetActiveObject(new MySimpleManagerClass());

            _manager.SelectProperty("Data");
            AssertNoNotifyChangedHappendAnymore();
        }

        [Test]
        public void TestSelectClassProperty_List()
        {
            var obj = new MySimpleManagerClass
            {
                Items = new List<MyListItem>
                {
                    new MyListItemA {Cxt = 1},
                    new MyListItemB {Cxt = 3},
                    new MyListItemA {Cxt = 9},
                }
            };

            SetActiveObject(obj);
            _calls.Clear();
            _manager.SelectProperty("Items");
            AssertNotifiyChangedActiveObjectHappend();
            AssertNotifiyIsBackEnabledHappend(true);
            AssertNotifiyIsNewItemEnabled(true);
            AssertNotifiyListTypes(new List<Type> {typeof(MyListItemA), typeof(MyListItemB) });
            AssertSelectedItemType(typeof(MyListItemA));
            AssertNoNotifyChangedHappendAnymore();
            AssertActiveObjectIsWrappedList(obj);

            _manager.NewItem(null);
            AssertNotifiyChangedActiveObjectHappend();
            AssertNotifiyIsBackEnabledHappend(true);
            AssertNotifiyIsNewItemEnabled(true);
            AssertNoNotifyChangedHappendAnymore();


        }

        private void AssertActiveObjectIsWrappedList(MySimpleManagerClass obj)
        {
            Assert.IsInstanceOf<WrappedList>(_manager.ActiveObject);
            var wrappedList = _manager.ActiveObject as WrappedList;
            var props = wrappedList.GetType().GetProperties();
            Assert.AreEqual(obj.Items.Count, props.Length);

            var i = 0;
            foreach (var info in props)
            {
                var propVal = info.GetMethod.Invoke(wrappedList, null);

                Assert.IsInstanceOf<MyListItem>(propVal);
                var cxt = typeof(MyListItem).GetProperty("Cxt").GetMethod.Invoke(propVal, null);
                Assert.AreEqual(obj.Items[i].Cxt, cxt);
                i++;
            }
        }

        private void SetActiveObject(object data)
        {
            _manager.ActiveObject = data;
            AssertNotifiyChangedActiveObjectHappend();
            AssertNotifiyIsBackEnabledHappend(false);
            AssertNotifiyIsNewItemEnabled(false);
            AssertNoNotifyChangedHappendAnymore();
        }

        private void AssertNotifiyIsBackEnabledHappend(bool b)
        {
            Assert.AreEqual("IsBackEnabled", _calls[0].PropertyName);
            Assert.AreEqual(b, _manager.IsBackEnabled);
            _calls.RemoveAt(0);
        }

        private void AssertSelectedItemType(Type exptectedType)
        {
            Assert.AreEqual(nameof(PropertyGridManager.SelectedItemType), _calls[0].PropertyName);
            Assert.AreEqual(exptectedType, _manager.SelectedItemType);
            _calls.RemoveAt(0);

        }
        private void AssertNotifiyListTypes(List<Type> exptectedlistType)
        {
            Assert.AreEqual(nameof(PropertyGridManager.ListTypes), _calls[0].PropertyName);
            Assert.AreEqual(exptectedlistType.Count, _manager.ListTypes.Count);
            for (int i = 0; i < exptectedlistType.Count; i++)
            {
                Assert.AreEqual(exptectedlistType[i], _manager.ListTypes[i]);
            }
            _calls.RemoveAt(0);
        }
        private void AssertNotifiyIsNewItemEnabled(bool value)
        {
            Assert.AreEqual(nameof(PropertyGridManager.IsNewListItemEnabled), _calls[0].PropertyName);
            Assert.AreEqual(value, _manager.IsBackEnabled);
            _calls.RemoveAt(0);
        }

        private void AssertNoNotifyChangedHappendAnymore()
        {
            Assert.AreEqual(0, _calls.Count);
        }

        private void AssertNotifiyChangedActiveObjectHappend()
        {
            Assert.AreEqual("ActiveObject", _calls[0].PropertyName);
            _calls.RemoveAt(0);
        }
    }
}
