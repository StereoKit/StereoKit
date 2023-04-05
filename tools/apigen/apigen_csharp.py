import os 
import re
import apigen

###########################################

def pyname(str):
    words  = str.split("_")
    pyName = ""
    for w in words:
        pyName += w.capitalize()

    return pyName

def pyname_t(str):
    words  = str.split("_")
    pyName = ""
    for w in words:
        if w is not "t":
            pyName += w.capitalize()

    return pyName

class BindPython:
    def __init__(self, enums, types, functions):
        self.enums     = enums
        self.types     = types
        self.functions = functions

    def __str__(self):
        result = "from ctypes import cdll\nfrom enum import Enum\n\nsklib = cdll.LoadLibrary('./StereoKitC.dll')\n"

        # Enums first
        for e in self.enums:
            eName = apigen.tocamel(e.name, True)
            result += "\nclass {}(Enum):\n".format(eName)
            val     = -1
            for p in e.properties:
                valText = ""
                if p[1] is None:
                    val = val + 1
                    valText = str(val)
                elif p[1].startswith(e.name):
                    words = p[1].split("|")
                    words = [apigen.tocamel(w.strip(), True)[len(eName):] for w in words]
                    valText = " | ".join(words)
                else:
                    valText = p[1]
                    if p[1].isnumeric():
                        val = int(p[1])
                    
                item = apigen.tocamel(p[0], True)
                item = item[len(eName):]
                result += "\t{} = {}\n".format(item, valText)

        # Types next
        for t in self.types:
            tName = apigen.tocamel(t.name, True)
            result += "\nclass {}:\n".format(tName)

            result += "\tdef __init__(self):\n"
            for p in t.properties:
                pName = apigen.tocamel(p[1], False)
                result += "\t\tself.{}\n".format(pName)

            result += "\n"
            for f in t.functions:
                result += "\tdef {}(self): {}\n".format(f.name, f.ret)
        return result

###########################################

def fcname(str, isVar):
    words  = str.split("_")
    pyName = ""
    for w in words:
        if pyName == "" and isVar == True:
            pyName += w
        else:
            pyName += w.capitalize()

    if pyName.isnumeric():
        pyName = "N"+pyName
    return pyName

def fcname_t(str):
    if str == "float": return "f32"
    if str == "double": return "f64"
    if str == "void": return "void"
    if str == "int": return "i32"
    if str == "uint": return "ui32"
    if str == "long": return "i64"
    if str == "ulong": return "ui64"
    if str == "int32_t": return "i32"
    if str == "uint32_t": return "ui32"
    if str == "int64_t": return "i64"
    if str == "uint64_t": return "ui64"
    if str == "int16_t": return "i16"
    if str == "uint16_t": return "ui16"
    if str == "int8_t": return "i8"
    if str == "uint8_t": return "ui8"
    if str == "bool32_t": return "bool"

    words  = str.split("_")
    pyName = ""
    for w in words:
        if w is not "t":
            pyName += w.capitalize()

    return pyName

class BindFerrC:
    def __init__(self, enums, types, functions):
        self.enums     = enums
        self.types     = types
        self.functions = functions

    def __str__(self):
        result = "module sk;\ninclude <stereokit.h>;\n"

        # Enums first
        for e in self.enums:
            eName = fcname(e.name, False)
            result += "\nenum {}@{} {{\n".format(eName, e.name)
            val     = -1
            for p in e.properties:
                result += "\t{}@{},\n".format(fcname(p[0][len(e.name):], False), p[0])
            result += "}"

        # Types next
        for t in self.types:
            tName = fcname_t(t.name)
            result += "\nstruct {}@{} {{\n".format(tName, t.name)

            for p in t.properties:
                newName = fcname(p[1], True)
                oldName = p[1]
                if newName != oldName:
                    result += "\t{} {}@{};\n".format(fcname_t(p[0]), newName, p[1])
                else:
                    result += "\t{} @{};\n".format(fcname_t(p[0]), p[1])

            if len(t.properties) > 0 and len(t.functions) > 0:
                result += "\n"
            for f in t.functions:
                fnName = ""
                if "_t" in t.name:
                    fnName = f.name[len(t.name)-1:]
                else:
                    fnName = f.name[len(t.name):]
                result += "\tfn {}@{}( -> {});\n".format(fcname(fnName, False), f.name, fcname_t(f.ret))
            result += "}"
        return result

###########################################

class BindCSharp:
    def __init__(self, enums, types, functions):
        self.enums     = enums
        self.types     = types
        self.functions = functions

    def typeName(param, isReturn):
        name = param.type

        if isReturn and param.array == 1: name = "IntPtr"
        elif name == "char" and param.array == 1: name = "string"
        elif name == "char" and param.array == 2: name = "string[]"
        elif name == "void" and (param.array > 0 or param.isRef == True): name = "IntPtr"
        elif name == "float": name = "float"
        elif name == "double": name = "double"
        elif name == "void": name = "void"
        elif name == "int": name = "int"
        elif name == "uint": name = "uint"
        elif name == "long": name = "long"
        elif name == "ulong": name = "ulong"
        elif name == "size_t": name = "ulong"
        elif name == "int32_t": name = "int"
        elif name == "uint32_t": name = "uint"
        elif name == "int64_t": name = "long"
        elif name == "uint64_t": name = "ulong"
        elif name == "int16_t": name = "short"
        elif name == "uint16_t": name = "ushort"
        elif name == "int8_t": name = "byte"
        elif name == "uint8_t": name = "byte"
        elif name == "bool32_t": name = "bool"
        # Some specific overrides
        elif name == "color128": name = "Color"
        elif name == "sk_settings_t": name = "SKSettings"
        elif name == "sh_light_t": name = "SHLight"
        elif name == "vert_t": name = "Vertex"
        elif name == "vind_t": name = "uint"
        elif name == "button_state_": name = "BtnState"
        elif name == "log_": name = "LogLevel"
        elif name == "gradient_t": name = "IntPtr"
        elif name == "mesh_t": name = "IntPtr"
        elif name == "tex_t": name = "IntPtr"
        elif name == "font_t": name = "IntPtr"
        elif name == "shader_t": name = "IntPtr"
        elif name == "material_t": name = "IntPtr"
        elif name == "solid_t": name = "IntPtr"
        elif name == "model_t": name = "IntPtr"
        elif name == "sprite_t": name = "IntPtr"
        elif name == "sound_t": name = "IntPtr"
        elif name == "material_buffer_t": name = "IntPtr"
        else: name = apigen.tocamel(name, True)
        return name

    def paramString(param, isReturn):
        prefix = ""
        postfix = ""
        if param.fnPtr is not None:
            return "{} {}".format(BindCSharp.delegateName(param.fnPtr), param.fnPtr.name)
            ptrType = ""
            ptrArgs = ""
            if param.fnPtr.ret.type == "void":
                ptrType = "Action"
            else:
                ptrType = "Func"
                ptrArgs = BindCSharp.typeName(param.fnPtr.ret, True)
            
            for p in param.fnPtr.params:
                if ptrArgs is not "":
                    ptrArgs += ", "
                ptrArgs += BindCSharp.typeName(p, False)

            if ptrArgs == "":
                return "{} {}".format(ptrType, param.fnPtr.name)
            else:
                return "{}<{}> {}".format(ptrType, ptrArgs, param.fnPtr.name)
        else:
            if not isReturn:
                if param.isRef:
                    if param.name.startswith("out_") and not param.isConst:
                        prefix = "out "
                    elif param.name.startswith("ref_") and not param.isConst:
                        prefix = "ref "
                    elif param.isConst:
                        prefix = "in "
                    else:
                        prefix = "ref "
                if param.array == 1 and param.type != "char" and param.type != "void":
                    if param.isConst:
                        prefix = "[In] "
                        postfix = "[]"
                    elif param.name.startswith("out_"):
                        prefix = "[Out] "
                        postfix = "[]"
                    else:
                        print("Not sure about this parameter: '{}'".format(param.srcText))
            name = param.name
            if name == "event":
                name = "evt"

            return "{}{}{} {}".format(prefix, BindCSharp.typeName(param, isReturn), postfix, name)

    def delegateName(fn):
        ptrType = ""
        ptrArgs = ""
        if fn.ret.type == "void":
            ptrType = "Del"
        else:
            ptrType = "DelRet"
            ptrArgs = BindCSharp.typeName(fn.ret, True)
        
        for p in fn.params:
            if ptrArgs is not "":
                ptrArgs += "_"
            ptrArgs += apigen.tocamel(p.name, True)

        if ptrArgs == "":
            return "{}".format(ptrType)
        else:
            return "{}_{}".format(ptrType, ptrArgs)

    def makeDelegate(param):
        params = ""
        valid  = True
        for p in param.fnPtr.params:
            if p.fnPtr is not None or p.variadic:
                valid = False # Don't really want to deal with variadic bindings, or recursive function pointers
            if params != "":
                params += ", "
            params += BindCSharp.paramString(p, False)

        delName = BindCSharp.delegateName(param.fnPtr)
        if valid:
            return (delName, "\t\t[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate {} {}({});\n".format(BindCSharp.typeName(param.fnPtr.ret, True), delName, params))
        else:
            return ("", "")

    def __str__(self):
        result = """// This file is generated by StereoKitRoot/tools/apigen.py, please don't 
// edit it by hand :)

using System;
using System.Runtime.InteropServices;
using System.Text;

namespace StereoKit
{
	internal static class NativeAPI
	{
		const string            dll  = "StereoKitC";
		const CharSet           cSet = CharSet.Ansi;
		const CallingConvention call = CallingConvention.Cdecl;

"""
        delegates = {}
        # Types next
        for f in self.functions:
            params = ""
            valid = True
            for p in f.params:
                if p.fnPtr is not None:
                    delegate = BindCSharp.makeDelegate(p)
                    delegates[delegate[0]] = delegate[1]
                if p.variadic:
                    valid = False # Don't really want to deal with variadic bindings
                if params != "":
                    params += ", "
                params += BindCSharp.paramString(p, False)

            if valid:
                result += "\t\t[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern "
                result += "{}({});\n".format(BindCSharp.paramString(f.ret, True), params)
        for d in delegates.values():
            result += d
        result += "\t}\n}\n"
        return result

###########################################

parse   = apigen.SKParse()
binding = BindCSharp(parse.skEnums, parse.skTypes, parse.skFunctions)
print(str(binding))