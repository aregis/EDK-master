using System;
using System.Collections;
using System.Collections.Generic;

namespace HueEffectEditor
{
    public class WrappedList
    {
        public void SetList(object list)
        {
            GetType().GetField("List").SetValue(this, list);
        }

        public object GetList()
        {
            return GetType().GetField("List").GetValue(this);
        }


        public void SetItemInProperty(int itemNr, object item)
        {
            GetType().GetProperty("Item" + itemNr).SetMethod.Invoke(this, new[] {item});
        }
    }
}