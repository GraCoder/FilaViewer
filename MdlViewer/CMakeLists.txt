cmake_minimum_required(VERSION 3.26.0)

include(CSharpUtilities)

set(target_name FilaMat)

project(${target_name} VERSION 8.0 LANGUAGES CSharp)

set(sources
	Program.cs
	ViewLocator.cs
)

add_executable(${target_name}
	${sources}
)

target_compile_options(${target_name} PRIVATE "/langversion:latest")

set_property(SOURCE App.axaml PROPERTY VS_XAML_TYPE "ApplicationDefinition")

set(vs_references
$<$<CONFIG:Debug>:"">
)

set_target_properties(${target_name} PROPERTIES
	VS_PACKAGE_REFERENCES "Avalonia_11.0.6;Avalonia.Desktop_11.0.6;Avalonia.Themes.Fluent_11.0.6;Avalonia.Fonts.Inter_11.0.6;Avalonia.ReactiveUI_11.0.6;Avalonia.Diagnostics_11.0.6"
)

set_target_properties(${target_name} PROPERTIES 
	DOTNET_SDK "Microsoft.NET.Sdk" 
	DOTNET_TARGET_FRAMEWORK "net8.0-windows" 
	#DONTNET_TARGET_FRAMEWORK_VERSION ".Net 8.0"
)

