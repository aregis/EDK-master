using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Reflection.Emit;
using System.Threading;
using huestream;
using Type = System.Type;

namespace HueEffectEditor
{
    public class WrappedListBuilder
    {
        public WrappedList Create(object list)
        {
            var assemblyBuilder = Thread.GetDomain()
                .DefineDynamicAssembly(new AssemblyName("DynamicAssembly"), AssemblyBuilderAccess.Run);
            var moduleBuilder = assemblyBuilder.DefineDynamicModule("DynamicAssembly.dll");
            var typeBuilder = moduleBuilder.DefineType(list.GetType().Name, TypeAttributes.Public, typeof(WrappedList));

            //var listType = typeof(List<>);
            //var constructedListType = listType.MakeGenericType(ListReflection.GetListItemType(list.GetType()));
            new DynamicFieldBuilder(typeBuilder).Create("List", list.GetType(), FieldAttributes.Public);

            CreateItemsAsProperties(list, typeBuilder);
            return SetValuesInList(list, typeBuilder.CreateType());
        }

        private static WrappedList SetValuesInList(object list, Type type)
        {
            var wrappedList = (WrappedList) Activator.CreateInstance(type);

            //var constructedListType = type.GetField("List").FieldType;
            //var listInstance = Activator.CreateInstance(constructedListType);
            type.GetField("List").SetValue(wrappedList, list);

            var i = 0;
            foreach (var item in list as IEnumerable)
            {
                wrappedList.SetItemInProperty(i, item);
                i++;
            }

            return wrappedList;
        }

        private static void CreateItemsAsProperties(object list, TypeBuilder typeBuilder)
        {
            var count = 0;
            foreach (var item in list as IEnumerable)
            {
                var name = "Item" + count;
                var realType = GetRealType(item);
                new DynamicPropertyBuilder(name, realType, typeBuilder).Create();
                count++;
            }
        }

        private static Type GetRealType(object item)
        {
            var type = item.GetType();
            var allDerivedTypes = ListReflection.FindAllDerivedTypes(type);
            if (allDerivedTypes.Count == 0)
                return type;

            foreach (var t in allDerivedTypes)
            {
                try
                {
                    if (type.UnderlyingSystemType == t)
                        return t;
                }
                catch (Exception)
                {
                    
                }
            }

            return type;

        }
    }
}