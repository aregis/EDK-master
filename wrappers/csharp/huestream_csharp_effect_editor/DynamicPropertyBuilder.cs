using System;
using System.Reflection;
using System.Reflection.Emit;

namespace HueEffectEditor
{
    class DynamicPropertyBuilder
    {
        private readonly TypeBuilder _typeBuilder;
        private FieldBuilder _fieldBuilder;
        private PropertyBuilder _propertyBuilder;
        private readonly string _propName;
        private readonly Type _propType;

        public DynamicPropertyBuilder(string propName, Type propType, TypeBuilder typeBuilder)
        {
            _propType = propType;
            _propName = propName;
            _typeBuilder = typeBuilder;
        }

        public void Create()
        {
            _propertyBuilder = _typeBuilder.DefineProperty(_propName, PropertyAttributes.HasDefault, _propType, null);
            CreatePropertyField();
            CreatePropertyGetMethod();
            CreatePropertySetMethod();
        }

        private void CreatePropertyField()
        {
            var firstCharLower = _propName.Substring(0, 1).ToLower();
            var fieldName = firstCharLower + _propName.Substring(1);
            _fieldBuilder = new DynamicFieldBuilder(_typeBuilder).Create(fieldName, _propType);
        }

        private void CreatePropertySetMethod()
        {
            var setterName = "set_" + _propName;
            var setPropMethodBuilder = _typeBuilder.DefineMethod(setterName,
                MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.HideBySig, null, new Type[] { _propType });
            var setIl = setPropMethodBuilder.GetILGenerator();
            setIl.Emit(OpCodes.Ldarg_0);
            setIl.Emit(OpCodes.Ldarg_1);
            setIl.Emit(OpCodes.Stfld, _fieldBuilder);
            setIl.Emit(OpCodes.Ret);
            _propertyBuilder.SetSetMethod(setPropMethodBuilder);
        }

        private void CreatePropertyGetMethod()
        {
            var getterName = "get_" + _propName;
            var getPropMethodBuilder = _typeBuilder.DefineMethod(getterName,
                MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.HideBySig, _propType, Type.EmptyTypes);
            var getIl = getPropMethodBuilder.GetILGenerator();
            getIl.Emit(OpCodes.Ldarg_0);
            getIl.Emit(OpCodes.Ldfld, _fieldBuilder);
            getIl.Emit(OpCodes.Ret);
            _propertyBuilder.SetGetMethod(getPropMethodBuilder);
        }
    }
}