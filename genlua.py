from lxml import etree
import lxml.html
import re
from collections import Counter
import os

valid_classes = [
    'bool',
    'body_part',
    'std::string',
    'units::mass',
    'units::volume',
    'int',
    'float',
    'item',
    'int',
    'itype',
]

class CppObj:
    def __init__(self):
        self.classes = []

    def getClass(self, name):
        for c in self.classes:
            if c.name == name:
                return c
        return None

    def appendClass(self, new_class):
        self.classes.append(new_class)

    def deleteClass(self, name):
        del_list = []
        for c in self.classes:
            if c.name == name:
                del_list.append(c)
        for c in del_list:
            self.classes.remove(c)

class CppFunction:
    def __init__(self, name = None):
        self.name = None
        self.args  = []
        self.optional_args = []
        self.rval = None

class CppClass:
    def __init__(self, name = None, string_id = None):
        self.name = name
        self.string_id = string_id
        self.is_valid = name in valid_classes
        self.functions = []

    def printLuaDefs(self):
        if not self.is_valid:
            print('--[[')
        print(self.name + ' = {')
        if self.string_id:
            print('    string_id = "' + self.string_id + '",')
        print('    functions = {')
        for function in self.functions:
            print('        ' + function)
        print('    },')
        print('},')
        if not self.is_valid:
            print(']]--')

def is_valid_class(str):
    for c in valid_classes:
        if c == str:
            return True
        if (c + '*') == str:
            return True
        if (c + '&') == str:
            return True
    return False

def is_valid_args(args):
    for arg in args:
        if arg not in valid_classes:
            return False
    return True

def get_xml_files(path):
    xml_files = []
    if os.path.isdir(path):
        files = os.listdir(path)
        for file in files:
            if os.path.isfile(path + '/' + file):
                xml_files.append(path + '/' + file)
    return xml_files

def update_class(cpp_obj, compound):
    class_name = compound.find('compoundname')
    target_class = cpp_obj.getClass(class_name.text)
    for member in compound.iter('memberdef'):
        if member.attrib['prot'] == 'public':
            if member.attrib['kind'] == 'function':
                function_name = "!!!invalid_function"
                args  = []
                optional_args = []
                rval = 'null'
                type = member.find('type')
                for name in member.iter('name'):
                    function_name = name.text
                if type != None:
                    tmp = etree.tostring(type, method="text")
                    tmp = re.sub(r'\s+$', '', tmp)
                    tmp = re.sub(r'const ', '', tmp)
                    tmp = re.sub(r'static ', '', tmp)
                    tmp = re.sub(r'const ', '', tmp)
                    tmp = re.sub(r'\*$', '&', tmp)
                    tmp = re.sub(r'\s+\&$', '&', tmp)
                    rval = tmp
                if rval == "":
                    if function_name == 'operator int':
                        rval = "int" 
                for param in member.iter('param'):
                    type = param.find('type')
                    if type != None:
                        tmp = etree.tostring(type, method="text")
                        tmp = re.sub(r'\s+$', '', tmp)
                        tmp = re.sub(r'^const ', '', tmp)
                        tmp = re.sub(r'^static ', '', tmp)
                        tmp = re.sub(r'^const ', '', tmp)
                        tmp = re.sub(r'const$', '', tmp)
                        tmp = re.sub(r'\s+[\*\&]$', '', tmp)
                        if param.find('defval') != None:
                            optional_args.append(tmp)
                        else:
                            args.append(tmp)
                string = "{ "
                string += 'name = "' + function_name + '", '
                string += 'rval = "' + rval + '", '
                string += 'args = { '
                for arg in args:
                    string += '"' + arg + '", '
                string += '}, '
                if len(optional_args):
                    string += 'optional_args = { '
                    for arg in optional_args:
                        string += '"' + arg + '", '
                    string += '}, '
                string += '}, '

                if is_valid_class(rval) and is_valid_args(args) and is_valid_args(optional_args):
                    target_class.functions.append(string)
                else:
                    target_class.functions.append('--' + string)

def update_string_id(cpp_obj, compound):
    members = tree.xpath('//memberdef')
    for member in members:
        if member.attrib['kind'] == 'typedef':
            definition = member.find('definition')
            if definition != None:
                tmp = etree.tostring(definition, method="text")
                tmp2 = re.findall(r'^using (\w+)\s+=\s+string_id<(\w+)>', tmp)
                if len(tmp2):
                    c = cpp_obj.getClass(tmp2[0][1])
                    if c:
                        c.string_id = tmp2[0][0]

cpp_obj = CppObj()

cpp_classes = {}

#
xml_files = get_xml_files('doxygen_doc/xml')

# Init Cpp Classes
for xml_file in xml_files:
    tree = etree.parse(xml_file)
    root = tree.getroot()

    compounds = tree.xpath('//compounddef')
    for compound in compounds:
        if compound.attrib['kind'] == 'class' or compound.attrib['kind'] == 'struct':
            class_name = compound.find('compoundname')
            cpp_obj.appendClass(CppClass(name = class_name.text))

# Update Classes
for xml_file in xml_files:
    tree = etree.parse(xml_file)
    root = tree.getroot()

    compounds = tree.xpath('//compounddef')
    for compound in compounds:
        if compound.attrib['kind'] == 'class':
            #Update Classes
            update_class(cpp_obj, compound)
        if compound.attrib['kind'] == 'file':
            #Update string_id
            update_string_id(cpp_obj, compound)

for c in cpp_obj.classes:
    c.printLuaDefs()

#{ name = "can_pickVolume", rval = "bool", args = { "item" }, optional_args = { "bool" } },

