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

parse   = apigen.SKParse()
binding = BindPython(parse.skEnums, parse.skTypes, parse.skFunctions)
print(str(binding))