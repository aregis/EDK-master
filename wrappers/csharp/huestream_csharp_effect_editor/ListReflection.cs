using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace HueEffectEditor
{
    public static class ListReflection
    {
        public static List<Type> FindAllDerivedTypes(Type type)
        {
            return FindAllDerivedTypes(type, Assembly.GetAssembly(type));
        }

        public static List<Type> FindAllDerivedTypes(Type type, Assembly assembly)
        {
            return assembly
                .GetTypes()
                .Where(t =>
                        !t.IsAbstract && HasDefaultConstructor(t) &&
                        type.IsAssignableFrom(t)
                ).ToList();

        }

        public static bool HasDefaultConstructor(this Type type)
        {
            return type.GetConstructor(Type.EmptyTypes) != null;
        }

        public static void AddNewDefaultItem(object list, Type type)
        {
            var listType = list.GetType();
            var newItem = Activator.CreateInstance(type);
            listType.GetMethod("Add").Invoke(list, new[] { newItem });
        }

        public static Type GetListItemType(Type listType)
        {
            var addMethod = listType.GetMethod("Add");
            var itemType = addMethod.GetParameters()[0].ParameterType;
            return itemType;
        }

        public static List<Type> GetDerivedItems(object list)
        {
            var listType = list.GetType();
            var itemType = GetListItemType(listType);
            return FindAllDerivedTypes(itemType);
        }
    }
}