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
        self.isStatic = True

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
    def __init__(self, name, isAsset):
        self.name       = name
        self.properties = []
        self.functions  = []
        self.isAsset    = isAsset

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
        if len(function.params) > 0:
            if function.params[0].type == self.name:
                function.isStatic = False
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

class SKParse:
    def __init__(self):
        dir_path    = os.path.dirname(os.path.realpath(__file__))
        file        = open(dir_path + "\\..\\..\\StereoKitC\\stereokit.h", "r")
        header_text = file.read()
        lines       = header_text.splitlines()

        self.skFunctions = []
        self.skEnums     = []
        self.skTypes     = []
        active = None

        self.skTypes.append(SKType("sk", False))

        for line in lines:
            if active is not None:
                if active.addline(line) is True:
                    active = None
            elif line.startswith("SK_API"):
                self.skFunctions.append(SKFunction(line))
            elif line.startswith("typedef enum"):
                active = SKEnum(line)
                self.skEnums.append(active)
            elif line.startswith("typedef struct"):
                active = SKType(re.search("struct\s+(.*)\s*{", line).group(1).strip(), False)
                self.skTypes.append(active)
            elif line.startswith("SK_DeclarePrivateType"):
                self.skTypes.append(SKType(re.search("\((.*)\)", line).group(1), True))

        for func in self.skFunctions:
            for type in self.skTypes:
                tName = type.name
                if type.name.endswith("_t"):
                    tName = type.name[:len(type.name)-1]
                if func.name.startswith(tName):
                    type.addfunc(func)