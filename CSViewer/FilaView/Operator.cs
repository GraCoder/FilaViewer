using System.Text.Json.Serialization;

internal class Operator
{
    public enum OperatorType
    {
        AddPrimitive = 1000,
        PickEntity
    }

    public OperatorType OperType { get; }

    public Operator(OperatorType opType)
    {
        OperType = opType;
    }
}

[JsonSerializable(typeof(string))]
[JsonSerializable(typeof(PrimitiveOperator))]
internal partial class OperatorSerializeContext : JsonSerializerContext
{

}