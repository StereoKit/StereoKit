import os 
import re
import apigen

###########################################

def cppname(str, isVar):
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

def cppname_t(type):
    if type == "float": return "f32"
    if type == "double": return "f64"
    if type == "void": return "void"
    if type == "int": return "i32"
    if type == "uint": return "ui32"
    if type == "long": return "i64"
    if type == "ulong": return "ui64"
    if type == "int32_t": return "i32"
    if type == "uint32_t": return "ui32"
    if type == "int64_t": return "i64"
    if type == "uint64_t": return "ui64"
    if type == "int16_t": return "i16"
    if type == "uint16_t": return "ui16"
    if type == "int8_t": return "i8"
    if type == "uint8_t": return "ui8"
    if type == "bool32_t": return "bool"

    words  = type.split("_")
    pyName = ""
    for w in words:
        if w is not "t":
            pyName += w.capitalize()

    return pyName

class BindCPP:
    def __init__(self, enums, types, functions):
        self.enums     = enums
        self.types     = types
        self.functions = functions

    def functionStr(t, f):
        fnName = ""
        params = ""
        if "_t" in t.name:
            fnName = f.name[len(t.name)-1:]
        else:
            fnName = f.name[len(t.name):]
        if not f.isStatic:
            params = "_inst"
        return "\t{}{} {}() {{ sk::{}({}); }}\n".format("static " if f.isStatic else "", cppname_t(f.ret.type), cppname(fnName, False), f.name, params)

    def __str__(self):
        result = "include <stereokit.h>;\n\nnamespace skpp {"

        # Enums first
        for e in self.enums:
            eName = cppname(e.name, False)
            result += "\nenum {}@{} {{\n".format(eName, e.name)
            val     = -1
            for p in e.properties:
                result += "\t{}@{},\n".format(cppname(p[0][len(e.name):], False), p[0])
            result += "};"

        # Types next
        for t in self.types:
            tName = cppname_t(t.name)
            result += "\nclass {} {{\n".format(tName)

            if t.isAsset:
                result += "private:\n\t{} _inst;\n\n\t{}({} inst) {{ _inst=inst; _inst = nullptr; }}\n\n".format(t.name, tName, t.name)
            result += "public:\n"
            if t.isAsset:
                result += "\t~{}() {{ sk::{}_release(_inst); }}\n\n".format(tName, t.name[0:-2])

            for p in t.properties:
                newName = cppname(p[1], True)
                oldName = p[1]
                if newName != oldName:
                    result += "\t{} {}@{};\n".format(cppname_t(p[0]), newName, p[1])
                else:
                    result += "\t{} @{};\n".format(cppname_t(p[0]), p[1])

            if len(t.properties) > 0 and len(t.functions) > 0:
                result += "\n"

            for f in t.functions:
                if not f.isStatic:
                    result += BindCPP.functionStr(t, f)
            result += "\n"
            for f in t.functions:
                if f.isStatic:
                    result += BindCPP.functionStr(t, f)

            result += "}"

        result += "\n\n} // namespace skpp"
        return result

###########################################

parse   = apigen.SKParse()
binding = BindCPP(parse.skEnums, parse.skTypes, parse.skFunctions)
print(str(binding))