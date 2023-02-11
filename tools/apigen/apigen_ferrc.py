import os 
import re
import apigen

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

def fcname_t(type):
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
                result += "\tfn {}@{}( -> {});\n".format(fcname(fnName, False), f.name, fcname_t(f.ret.name))
            result += "}"
        return result

###########################################

parse   = apigen.SKParse()
binding = BindFerrC(parse.skEnums, parse.skTypes, parse.skFunctions)
print(str(binding))