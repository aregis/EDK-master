%pragma(csharp) moduleclassmodifiers=%{
     internal class TypeMap 
    {
        internal static TypeMap Instance;

        private readonly System.Collections.Generic.Dictionary<string, System.Type> _admin;

        static TypeMap()
        {
            Instance = new TypeMap();

        }

        static System.Collections.Generic.List<System.Type> FindAllDerivedTypes(System.Type type)
        {
            var l = new System.Collections.Generic.List<System.Type>();
            foreach (var t in System.Reflection.Assembly.GetAssembly(type).GetTypes())
            {
                if (!t.IsAbstract && type.IsAssignableFrom(t))
                {
                    l.Add(t);
                }
                
            }
            return l;
        }
        public static bool HasDefaultConstructor(System.Type type)
        {
            return type.GetConstructor(System.Type.EmptyTypes) != null;
        }

        internal TypeMap()
        {
            _admin = new System.Collections.Generic.Dictionary<string, System.Type>();
            var types = FindAllDerivedTypes(typeof(Serializable));
            foreach (var type in types)
            {
                _admin.Add("huestream." + type.Name, type);
            }
        }

        public System.IntPtr GetSwigIntPtr(object o)
        {
            var type = o.GetType();
            var info = type.GetMethod("getCPtr", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Static);
            return ((System.Runtime.InteropServices.HandleRef)info.Invoke(null, new object[] {o})).Handle;
        }

        public T Upcast<T>(System.IntPtr cPtr)
        {
            var obj = CallSwigConstructor<T>(cPtr, false) as Serializable;
            if (obj == null)
                return CallSwigConstructor<T>(cPtr, true);
            var typeName = obj.GetTypeName();
            var type = GetType(typeName);
            return (T)CallSwigConstructor(type, cPtr, true);
        }

        private System.Type GetType(string typeName)
        {

            return _admin[typeName];
        }

        public T CallSwigConstructor<T>(System.IntPtr cPtr, bool ownMemory)
        {
            var type = typeof(T);
            var newObj = CallSwigConstructor(type, cPtr, ownMemory);
            return (T)newObj;
        }

        private static object CallSwigConstructor(System.Type type, System.IntPtr cPtr, bool ownMemory)
        {
            var info = type.GetConstructor(
                System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance,
                null, new System.Type[] {typeof(System.IntPtr), typeof(bool)}, null);
            var newObj = info.Invoke(new object[] {cPtr, ownMemory});
            return newObj;
        }

        public static T CreateInstance<T>(params object[] args)
        {
            var type = typeof(T);
            var instance = type.Assembly.CreateInstance(
                type.FullName, false,
                System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic,
                null, args, null, null);
            return (T)instance;
        }

    }

public class%}