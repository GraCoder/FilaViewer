class PrimitiveOperator : Operator
{
    public enum PrimitiveType
    {
        Cube, 
        Sphere
    }

    public PrimitiveType PrimType { get; }

    public PrimitiveOperator(PrimitiveType primitiveType)
        : base(OperatorType.AddPrimitive)
    {
        PrimType = primitiveType;
    }
}