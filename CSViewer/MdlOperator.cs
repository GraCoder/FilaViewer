using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json.Serialization;

namespace MdlViewer;

// This attribute tells the generator to write the serialization code for 'User'
[JsonSerializable(typeof(AddPrimitive))]
internal partial class OperJsonContext : JsonSerializerContext
{
}

public record AddPrimitive(string Name, float Rad);
