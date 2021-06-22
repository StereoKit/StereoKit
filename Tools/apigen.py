import os 
import re

###########################################

def tocamel(str, uppercaseStart):
    words  = str.split("_")
    result = ""
    for w in words:
        if w is "t":
            continue
        elif result == "" and uppercaseStart == False:
            result += w
        else:
            result += w.capitalize()

    if result.isnumeric():
        if uppercaseStart:
            result = "N"+result
        else:
            result = "n"+result

    return result

###########################################

class SKFunction:
    def __init__(self, text):
        self.srcText = text
        matches = re.search("SK_API\s+(.[^\(]*)\s*\((.*)\);", text)
        start   = matches[1]
        params  = matches[2]
        nameMatches = re.search("(.*?\s*\**)(\w*)\s*$", start)
        self.name = nameMatches[2].strip()
        retString  = nameMatches[1].strip()
        self.ret = SKParam(retString + " " + self.name)
        self.params = []

        currParam = ""
        parens = 0
        for c in params:
            if   c == '(': parens += 1
            elif c == ')': parens -= 1
            
            if c == ',' and parens == 0:
                self.params.append(SKParam(currParam.strip()))
                currParam = ""
            else:
                currParam += c
        if currParam != "":
            self.params.append(SKParam(currParam.strip()))


    def __str__(self):
        return self.srcText

###########################################

class SKParam:
    def __init__(self, text):
        self.srcText  = text
        self.type     = ""
        self.name     = ""
        self.val      = None
        self.variadic = False
        self.fnPtr    = None
        self.isConst  = False
        self.isRef    = False
        self.array    = 0

        matches = re.search("(.*?)\\b(\w+)\s*(?:sk_default\((.*)\)$|$)", text)
        if matches:
            self.type = matches[1]
            self.name = matches[2]
            self.val  = matches[3]

            if self.type: self.type = self.type.strip()
            if self.name: self.name = self.name.strip()
            if self.val:  self.val  = self.val .strip()
        else:
            if text == '...':
                self.variadic = True
            elif "(*" in text.replace(" ", ""):
                sections = re.search("(.*)\(\s*\*(.*?)\)(.*)", text)
                self.fnPtr = SKFunction("SK_API " + sections[1] + sections[2] + sections[3] + ";")
            else:
                print("Bad matches for '"+text+"'")

        # const status
        self.isConst = self.type.startswith("const")
        if self.isConst:
            self.type = self.type[len("const"):].strip()

        # sk_ref status
        self.isRef = self.type.startswith("sk_ref(")
        if self.isRef:
            self.type = self.type[len("sk_ref("):len(self.type)-1].strip()

        # sk_ref_arr status
        self.array = 0
        if self.type.startswith("sk_ref_arr("):
            self.array += 1
            self.type = self.type[len("sk_ref_arr("):len(self.type)-1].strip()

        # pointer status
        while self.type.endswith("*"):
            self.array += 1
            self.type = self.type[:len(self.type)-1].strip()

###########################################

class SKType:
    def __init__(self, name):
        self.name       = name
        self.properties = []
        self.functions  = []

    def __str__(self):
        result = self.name + ":\n"
        for p in self.properties:
            result += "\t" + p[0] + " " + p[1] + "\n"
        return result

    def addline(self, text):
        matches = re.search("\s*(\w*)\s+(\w*);", text)
        if matches is not None and matches[1] is not None and matches[1] is not "":
            self.properties.append((matches[1].strip(), matches[2].strip()))
        return "}" in text

    def addfunc(self, function):
        self.functions.append(function)

###########################################

class SKEnum:
    def __init__(self, text):
        self.name       = re.search("enum\s+(.*)\s*{", text).group(1).strip()
        self.properties = []

    def __str__(self):
        result = self.name + ":\n"
        for p in self.properties:
            if p[1] is None:
                result += "\t" + p[0] + "\n"
            else:
                result += "\t" + p[0] + " = " + p[1] + "\n"
        return result

    def addline(self, text):
        words = text.split(",")
        for word in words:
            matches = re.search("\s*(\w*)\s*(=\s*(.[^,]*))?", word)
            if matches[1] is not None and matches[1] is not "":
                self.properties.append((matches[1], matches[3]))
        return "}" in text

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
            eName = tocamel(e.name, True)
            result += "\nclass {}(Enum):\n".format(eName)
            val     = -1
            for p in e.properties:
                valText = ""
                if p[1] is None:
                    val = val + 1
                    valText = str(val)
                elif p[1].startswith(e.name):
                    words = p[1].split("|")
                    words = [tocamel(w.strip(), True)[len(eName):] for w in words]
                    valText = " | ".join(words)
                else:
                    valText = p[1]
                    if p[1].isnumeric():
                        val = int(p[1])
                    
                item = tocamel(p[0], True)
                item = item[len(eName):]
                result += "\t{} = {}\n".format(item, valText)

        # Types next
        for t in self.types:
            tName = tocamel(t.name, True)
            result += "\nclass {}:\n".format(tName)

            result += "\tdef __init__(self):\n"
            for p in t.properties:
                pName = tocamel(p[1], False)
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

    def typeName(param):
        name = param.type
        if   name == "char" and param.array == 1: name = "string"
        elif name == "char" and param.array == 2: name = "string[]"
        elif name == "void" and (param.array == 1 or param.isRef == True): name = "IntPtr"
        elif name == "float": name = "float"
        elif name == "double": name = "double"
        elif name == "void": name = "void"
        elif name == "int": name = "int"
        elif name == "uint": name = "uint"
        elif name == "long": name = "long"
        elif name == "ulong": name = "ulong"
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
        else: name = tocamel(name, True)
        return name

    def paramString(param):
        prefix = ""
        postfix = ""
        if param.fnPtr is not None:
            ptrType = ""
            ptrArgs = ""
            if param.fnPtr.ret.type == "void":
                ptrType = "Action"
            else:
                ptrType = "Func"
                ptrArgs = BindCSharp.typeName(param.fnPtr.ret)
            
            for p in param.fnPtr.params:
                if ptrArgs is not "":
                    ptrArgs += ", "
                ptrArgs += BindCSharp.typeName(p)

            if ptrArgs == "":
                return "{} {}".format(ptrType, param.fnPtr.name)
            else:
                return "{}<{}> {}".format(ptrType, ptrArgs, param.fnPtr.name)
        else:
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

            return "{}{}{} {}".format(prefix, BindCSharp.typeName(param), postfix, name)

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

        # Types next
        for f in self.functions:
            params = ""
            valid = True
            for p in f.params:
                if p.variadic:
                    valid = False # Don't really want to deal with variadic bindings
                if params != "":
                    params += ", "
                params += BindCSharp.paramString(p)

            if valid:
                result += "\t\t[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern "
                result += "{}({});\n".format(BindCSharp.paramString(f.ret), params)
        result += "\t}\n}\n"
        return result

###########################################

dir_path = os.path.dirname(os.path.realpath(__file__))

file        = open(dir_path + "\\..\\StereoKitC\\stereokit.h", "r")
header_text = file.read()
lines       = header_text.splitlines()

skFunctions = []
skEnums     = []
skTypes     = []
active = None

skTypes.append(SKType("sk"))

for line in lines:
    if active is not None:
        if active.addline(line) is True:
            active = None
    elif line.startswith("SK_API"):
        skFunctions.append(SKFunction(line))
    elif line.startswith("typedef enum"):
        active = SKEnum(line)
        skEnums.append(active)
    elif line.startswith("typedef struct"):
        active = SKType(re.search("struct\s+(.*)\s*{", line).group(1).strip())
        skTypes.append(active)
    elif line.startswith("SK_DeclarePrivateType"):
        skTypes.append(SKType(re.search("\((.*)\)", line).group(1)))

for func in skFunctions:
    for type in skTypes:
        tName = type.name
        if type.name.endswith("_t"):
            tName = type.name[:len(type.name)-1]
        if func.name.startswith(tName):
            type.addfunc(func)

binding = BindCSharp(skEnums, skTypes, skFunctions)
print(str(binding))