using System;
using System.Reflection;
using System.Reflection.Emit;

namespace HueEffectEditor
{
    class DynamicFieldBuilder
    {
        private readonly TypeBuilder _typeBuilder;
        public DynamicFieldBuilder(TypeBuilder typeBuilder)
        {
            _typeBuilder = typeBuilder;
        }

        public FieldBuilder Create(string name, Type type, FieldAttributes attributes = FieldAttributes.Private)
        {
            return _typeBuilder.DefineField(name, type, attributes);
        }

    }
}