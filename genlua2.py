from lxml import etree
import lxml.html
import re
import os

valid_types = [
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
    'item_location',
    'player',
    'gun_mode',
    'effect_type',
    'calendar',
    'mutation_branch',
    'Character',
    'item_stack_iterator',
    'map_stack',
    'game',
    'activity_type',
    'player_activity',
    'bionic',
    'bionic_data',
    'morale_type_data',
    'encumbrance_data',
    'stats',
    'npc_companion_mission',
    'npc_personality',
    'npc_opinion',
    'npc',
    'point',
    'tripoint',
    'uilist',
    'field_entry',
    'field',
    'map',
    'ter_t',
    'furn_t',
    'Creature',
    'monster',
    'recipe',
    'martialart',
    'material_type',
    'start_location',
    'ma_buff',
    'ma_technique',
    'Skill',
    'quality',
    'npc_template',
    'species_type',
    'ammunition_type',
    'MonsterGroup',
    'mtype',
    'mongroup',
    'overmap',
    'nc_color',
    'time_duration',
    'time_point',
    'trap',
    'w_point',
    'void',
    'long',
    'double',
]

#=======================================================
#
# CppFunction
#
#=======================================================
class CppFunction:
    def __init__(self):
        self.name = None
        self.args  = []
        self.optional_args = []
        self.type = None

    @classmethod
    def load_from_xml(cls, xml_memberdef):
        if xml_memberdef.attrib['prot'] == 'public':
            if xml_memberdef.attrib['kind'] == 'function':
                new_func = CppFunction()
                new_func.name = xml_memberdef.find('name').text
                new_func.type = etree.tostring(xml_memberdef.find('type'), method="text")
                for p in xml_memberdef.iter('param'):
                    s = etree.tostring(p.find('type'), method="text")
                    if p.find('defval') != None:
                        new_func.optional_args.append(s)
                    else:
                        new_func.args.append(s)
                return new_func
        return None

    def str(self):
        string = "{ "
        string += 'name = "' + self.name + '", '
        string += 'rval = '
        if CppType(self.type).lua_rval() == "null":
            string += 'nil, '
        else:
            string += '"' + CppType(self.type).lua_rval() + '", '
        string += 'args = { '
        for a in self.args:
            string += '"' + CppType(a).lua_arg() + '", '
        string += '}, '
        if len(self.optional_args):
            string += 'optional_args = { '
            for a in self.optional_args:
                string += '"' + CppType(a).lua_arg() + '", '
            string += '}, '
        string += '}'
        return string

    def isValid(self):
        if not CppType(self.type).isValid():
            return False
        for a in self.args:
            if not CppType(a).isValid():
                return False
        for a in self.optional_args:
            if not CppType(a).isValid():
                return False
        return True

#=======================================================
#
# CppVariable
#
#=======================================================
class CppVariable:
    def __init__(self):
        self.name = None
        self.type = None

    @classmethod
    def load_from_xml(cls, xml_memberdef):
        if xml_memberdef.attrib['prot'] == 'public':
            if xml_memberdef.attrib['kind'] == 'variable':
                new_var = CppVariable()
                new_var.name = xml_memberdef.find('name').text
                new_var.type = etree.tostring(xml_memberdef.find('type'), method="text")
                return new_var
        return None

    def str(self):
        string = self.name + ' = { '
        string += 'type = ' + CppType(self.type).lua_arg() + ', '
        string += 'writable = '
        if self.isWritable():
            string += 'true, '
        else:
            string += 'false, '
        string += '}'
        return string

    def isWritable(self):
        t = CppType(self.type)
        if t.is_const or t.is_static:
            return False
        return True

    def isValid(self):
        if not CppType(self.type).isValid():
            return False
        return True

#=======================================================
#
# CppType
#
#=======================================================
class CppType:
    lua_typemap = {
        'std::string': 'string',
        'void': 'null',
        'long': 'int',
        'double': 'float',
    }

    def __init__(self, definition):
        self.definition = definition
        self.is_static = False
        self.is_const = False
        self.is_ptr = False
        self.is_ref = False
        self.name = ''
        self.parseCppDefinition()

    def parseCppDefinition(self):
        s = self.definition
        s = re.sub(r'\s+$', '', s)
        self.is_static = re.match(r'^static ', s)
        s = re.sub(r'^static ', '', s)
        self.is_const = re.match(r'^const ', s)
        s = re.sub(r'^const ', '', s)
        self.is_ptr = re.match(r'\s+\*$', s)
        s = re.sub(r'\s+\*$', '', s)
        self.is_ref = re.match(r'\s+\&$', s)
        s = re.sub(r'\s+\&$', '', s)
        if not self.is_const:
            self.is_const = re.match(r'\s+const$', s)
        s = re.sub(r'\s+const$', '', s)
        self.name = s

    def isWritable(self):
        if self.is_const:
            return False
        return True

    def lua_rval(self):
        s = self.name
        if s in CppType.lua_typemap.keys():
            s = CppType.lua_typemap[s]
        if self.is_ref or self.is_ptr:
            s += '&'
        return s

    def lua_arg(self):
        s = self.name
        if s in CppType.lua_typemap.keys():
            s = CppType.lua_typemap[s]
        return s

    def isValid(self):
        return self.name in valid_types

#=======================================================
#
# CppClass
#
#=======================================================
class CppClass:
    def __init__(self):
        self.name = None
        self.string_id = None
        self.attributes = []
        self.functions = []

    @classmethod
    def load_from_xml(cls, xml_compounddef):
        new_class = CppClass()
        new_class.name = xml_compounddef.find('compoundname').text
        #function
        for member in xml_compounddef.iter('memberdef'):
            new_func = CppFunction.load_from_xml(member)
            if new_func:
                new_class.functions.append(new_func)

            new_attribute = CppVariable.load_from_xml(member)
            if new_attribute:
                new_class.attributes.append(new_attribute)
        return new_class

    def str(self):
        s = self.name + ' = {' + '\n'
        if self.string_id:
            s += '    string_id = ' + self.string_id + '\n'
        s += '    new = {' + '\n'
        for f in self.functions:
            if self.name == f.name:
                s += '        '
                if not f.isValid():
                    s += '--'
                s += '{ '
                for a in f.args:
                    s += CppType(a).lua_arg() + ', '
                s += '}\n'
        s += '    },' + '\n'
        s += '    attributes = {' + '\n'
        for a in self.attributes:
            s += '        '
            if not a.isValid():
                s += '--'
            s += a.str() + '\n'
        s += '    },' + '\n'
        s += '    functions = {' + '\n'
        for f in self.functions:
            s += '        '
            if not f.isValid():
                s += '--'
            s += f.str() + ',\n'
        s += '    }' + '\n'
        s += '}, '
        return s

def get_xml_files(path):
    xml_files = []
    if os.path.isdir(path):
        files = os.listdir(path)
        for file in files:
            if os.path.isfile(path + '/' + file):
                xml_files.append(path + '/' + file)
    return xml_files

xml_files = get_xml_files('doxygen_doc/xml')
all_cpp_classes = []
string_ids = {}
int_ids = {}
typedefs = {}

for xml_file in xml_files:
    tree = etree.parse(xml_file)
    root = tree.getroot()

    compounds = tree.xpath("//compounddef[@kind='class']")
    # Enumurate all types
    for compound in compounds:
        new_class = CppClass.load_from_xml(compound)
        all_cpp_classes.append(new_class)

    # Generate typedefs
    members = tree.xpath("//memberdef[@kind='typedef']")
    for member in members:
        definition = member.find('definition')
        tmp = etree.tostring(definition, method="text")
        tmp2 = re.findall(r'^using\s+(\w+)\s+=\s+(.+)', tmp)
        if len(tmp2):
            typedefs[tmp2[0][0]] = tmp2[0][1]


    # string_id
    members = tree.xpath("//memberdef[@kind='typedef']")
    for member in members:
        definition = member.find('definition')
        tmp = etree.tostring(definition, method="text")
        tmp2 = re.findall(r'^using (\w+)\s+=\s+string_id<([\w:]+)>', tmp)
        if len(tmp2):
            string_id_class = tmp2[0][1]
            string_id_type = tmp2[0][0]
            string_ids[string_id_class] = string_id_type

    # int_id
    members = tree.xpath("//memberdef[@kind='typedef']")
    for member in members:
        definition = member.find('definition')
        tmp = etree.tostring(definition, method="text")
        tmp2 = re.findall(r'^using (\w+)\s+=\s+int_id<([\w:]+)>', tmp)
        if len(tmp2):
            int_id_class = tmp2[0][1]
            int_id_type = tmp2[0][0]
            int_ids[string_id_class] = int_id_type

for si in string_ids.keys():
    if string_ids[si] not in valid_types:
        valid_types.append(string_ids[si])

for ii in int_ids.keys():
    if int_ids[ii] not in valid_types:
        valid_types.append(int_ids[ii])

for c in all_cpp_classes:
    print c.str()

print typedefs
