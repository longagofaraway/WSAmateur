import sys

def readFileWithoutComments(f):
    doc = ''
    for line in f:
        splitlines = line.split('//')
        doc += splitlines[0] + '\n'
    return doc

class CppDoc:
    def __init__(self):
        self.hpp = ''
        self.headers = ''
        self.declarations = ''
        self.definitions = ''
        self.variantMap = {}
        self.variantCode = {}
        self.variantType = {}

cpp = CppDoc()

def addCommonTypes(cpp):
    cpp.hpp += '''
#include <string>

namespace asn {
struct Ability;
}

std::string serializeAbility(const asn::Ability& field);
'''
    cpp.headers += '''
#include <string>
#include <functional>
#include <unordered_map>

#include "abilities.h"

using namespace asn;
'''
    cpp.declarations += '''
std::string serializeInt8(int field, int);
std::string serializeUInt8(unsigned int field, int);
std::string serializeInt32(int field, int);
std::string serializeUInt32(unsigned int field, int);
std::string serializeString(std::string field, int);
std::string serializeBool(bool field, int);
struct JsonRecord {
    std::string key;
    std::string value;
};
using JsonRecords = std::vector<JsonRecord>;
'''
    cpp.definitions += '''
std::string serializeInt8(int field, int) {
    return std::to_string(field);
}
std::string serializeUInt8(unsigned int field, int) {
    return std::to_string(field);
}
std::string serializeInt32(int field, int) {
    return std::to_string(field);
}
std::string serializeUInt32(unsigned int field, int) {
    return std::to_string(field);
}
std::string serializeString(std::string field, int) {
    return "\\"" + field + "\\"";
}
std::string serializeBool(bool field, int) {
    return field ? "true" : "false";
}
std::string printRecords(const JsonRecords& records, int offset) {
    std::string res;
    for (size_t i = 0; i < records.size(); ++i) {
        for (int j = 0; j < offset; ++j)
            res += " ";
        res += "\\"" + records[i].key + "\\": ";
        res += records[i].value;
        if (i < records.size() - 1)
            res += ",\\n";
    }
    res += "\\n";
    return res;
}
'''


def getVariantType(fieldType, cpp):
    for key in cpp.variantMap:
        if fieldType in cpp.variantMap[key]:
            return cpp.variantType[key]
    return None


def parseEnum(tokens, current, cpp):
    fieldType = tokens[current]
    variantType = getVariantType(fieldType, cpp)
    code = ''
    if variantType:
        cpp.declarations += 'std::string serialize{fieldType}(const {variantType}& fieldVariant, int offset);\n'.format(fieldType=fieldType, variantType=variantType)
        code += '''\nstd::string serialize{fieldType}(const {variantType}& fieldVariant, int offset) {{
    const auto& field = std::get<{fieldType}>(fieldVariant);
    return std::to_string(static_cast<int>(field));
}}\n'''.format(fieldType=fieldType, variantType=variantType)
    cpp.declarations += 'std::string serialize{fieldType}({fieldType} field, int);\n'.format(fieldType=fieldType)
    code += '''std::string serialize{fieldType}({fieldType} field, int) {{
    return std::to_string(static_cast<int>(field));
}}\n
'''.format(fieldType=fieldType)
    cpp.definitions += code
    while tokens[current][0] != '}':
        current += 1
    return current + 1


def addToVariantMap(fieldType, memberName, tokens, current, cpp):
    cpp.variantMap[fieldType] = {}
    cpp.variantType[fieldType] = 'decltype({fieldType}::{memberName})'.format(fieldType=fieldType, memberName=memberName)
    typeMap = cpp.variantMap[fieldType]
    # '{'
    current += 1
    variantValue = tokens[current].replace(',', '')
    i = 1
    cpp.variantCode[fieldType] = '''
std::unordered_map<size_t,std::function<std::string(const decltype({fieldType}::{memberName})&, int)>> {lowerFieldType}SerializerMap = {{'''\
.format(fieldType=fieldType, memberName=memberName, lowerFieldType=fieldType.lower())
    while variantValue != 'Nothing' and variantValue != '}':
        typeMap[variantValue] = i

        cpp.variantCode[fieldType] += '''
    {{{i}, static_cast<std::string(*)(const {variantType}&, int)>(serialize{variantValue})}}'''\
    .format(i=i, variantValue=variantValue, variantType=cpp.variantType[fieldType])

        i += 1
        current += 1
        variantValue = tokens[current].replace(',', '')
        if variantValue != 'Nothing' and variantValue != '}':
            cpp.variantCode[fieldType] += ','
        else:
            cpp.variantCode[fieldType] += '\n};\n'

    while tokens[current][0] != '}':
        current += 1
    current += 1
    return current


def parseVariant(fieldType, memberName, tokens, current, cpp):
    current = addToVariantMap(fieldType, memberName, tokens, current, cpp)
    
    code = '''\n\
    size_t index = field.{memberName}.index();
    if ({lowerFieldType}SerializerMap.contains(index)) {{
        JsonRecord record{memberName};
        record{memberName}.key = "{memberName}";
        record{memberName}.value = {lowerFieldType}SerializerMap[index](field.{memberName}, offset);
        records.push_back(record{memberName});
    }}
'''.format(lowerFieldType=fieldType.lower(), memberName=memberName)

    return code, current


def parseChoice(fieldType, memberName, tokens, current, cpp):
    choiceType = ''
    indirection = ''
    if fieldType == 'ForEachMultiplier' and memberName == 'place':
        choiceType = 'Place'
        code = '''\
    if (field.placeType == PlaceType::SpecificPlace) {
'''
    elif fieldType == 'ForEachMultiplier' and memberName == 'markerBearer':
        choiceType = 'Target'
        indirection = '*'
        code = '''\
    if (field.placeType == PlaceType::Marker) {
'''
    elif fieldType == 'Target':
        choiceType = 'TargetSpecificCards'
        code = '''\
    if (field.type == TargetType::SpecificCards || field.type == TargetType::BattleOpponent) {
'''
    elif fieldType == 'AttributeGain':
        choiceType = 'Multiplier'
        code = '''\
    if (field.gainType == ValueType::Multiplier) {
'''
    elif fieldType == 'Look':
        choiceType = 'Multiplier'
        code = '''\
    if (field.valueType == ValueType::Multiplier) {
'''
    elif fieldType == 'RevealCard':
        choiceType = 'Card'
        code = '''\
    if (field.type == RevealType::FromHand) {
'''
    elif fieldType == 'DealDamage':
        choiceType = 'Multiplier'
        code = '''\
    if (field.damageType == ValueType::Multiplier) {
'''
    elif fieldType == 'TargetAndPlace':
        choiceType = 'Place'
        code = '''\
    if (field.placeType == PlaceType::SpecificPlace) {
'''
    elif fieldType == 'AutoAbility':
        choiceType = 'Cost'
        code = '''\
    if (field.cost.has_value()) {
'''
    elif (fieldType in ['Effect', 'Trigger', 'CardSpecifier', 'Condition', 
                        'CostItem', 'Ability', 'Multiplier']):
        return parseVariant(fieldType, memberName, tokens, current, cpp)
    else:
        raise Exception('Unknown Choice type' + fieldType)
    if len(choiceType) > 0:
        code += '''\
        JsonRecord record{memberName};
        record{memberName}.key = "{memberName}";
        record{memberName}.value = serialize{choiceType}({indirection}field.{memberName}.value(), offset);
        records.push_back(record{memberName});
    }}
'''.format(memberName=memberName, choiceType=choiceType, indirection=indirection)
    while tokens[current][0] != '}':
        current += 1
    return code, current + 1


def parseArray(memberName, tokens, current):
    #skip 'of'
    current += 1
    memberType = tokens[current].replace(',', '')
    code = '''\
    JsonRecord record{memberName};
    record{memberName}.key = "{memberName}";
    record{memberName}.value = "[";
    for (size_t i = 0; i < field.{memberName}.size(); ++i) {{
        record{memberName}.value += serialize{memberType}(field.{memberName}[i], offset);
        if (i < field.{memberName}.size() - 1)
            record{memberName}.value += ", ";
    }}
    record{memberName}.value += "]";
    records.push_back(record{memberName});
'''.format(memberName=memberName, memberType=memberType)
    return code, current + 1


def parseStruct(tokens, current, cpp):
    fieldType = tokens[current]
    code = ''
    variantType = getVariantType(fieldType, cpp)
    code_add, current = addStructBody(fieldType, tokens, current, cpp)
    if variantType:
        cpp.declarations += 'std::string serialize{fieldType}(const {variantType}& fieldVariant, int offset);\n'.format(fieldType=fieldType, variantType=variantType)
        code = '''\nstd::string serialize{fieldType}(const {variantType}& fieldVariant, int offset) {{
    const auto& field = std::get<{fieldType}>(fieldVariant);'''.format(fieldType=fieldType, variantType=variantType)
        code += code_add

    cpp.declarations += 'std::string serialize{fieldType}(const {fieldType}& field, int offset);\n'.format(fieldType=fieldType)
    code += '''\nstd::string serialize{fieldType}(const {fieldType}& field, int offset) {{'''.format(fieldType=fieldType)
    code += code_add

    cpp.definitions += code
    return current

def addStructBody(fieldType, tokens, current, cpp):
    code = '''
    std::string r = "{\\n";
    offset += 4;
    JsonRecords records;
'''

    current += 2
    while tokens[current] != '}':
        memberName = tokens[current]
        current += 1
        memberType = tokens[current].replace(',', '')
        current += 1

        if memberType == 'Choice':
            choice_code, current = parseChoice(fieldType, memberName, tokens, current, cpp)
            code += choice_code
        elif memberType == 'Array':
            array_code, current = parseArray(memberName, tokens, current)
            code += array_code
        else:
            indirection = ''
            if memberName == 'target' and (fieldType == 'ForEachMultiplier' or 
                                           fieldType == 'AddLevelMultiplier' or
                                           fieldType == 'AddTriggerNumberMultiplier'):
                indirection = '*'
            if memberName == 'ability' and fieldType == 'DelayedAbility':
                indirection = '*'
            if memberName == 'effect' and fieldType == 'CostSubstitution':
                indirection = '*'
                
            code += '''\n\
    JsonRecord record{memberName};
    record{memberName}.key = "{memberName}";
    record{memberName}.value = serialize{memberType}({indirection}field.{memberName}, offset);
    records.push_back(record{memberName});
'''.format(memberType=memberType, memberName=memberName, indirection=indirection)
    code += '''\n\
    r += printRecords(records, offset);
    offset -= 4;
    for (int i = 0; i < offset; ++i)
        r += " ";
    r += "}";
    return r;
}\n\n'''
    return code, current + 1


def getVariantCode(variantCode):
    code = ''
    for key in variantCode:
        code += variantCode[key]
    return code

addCommonTypes(cpp)

def parseDocFiles(files):
    for filename in files:
        f = open(filename, 'r')
        doc = readFileWithoutComments(f)
        tokens = doc.split()

        cur_i = 0
        while cur_i < len(tokens):
            if (tokens[cur_i] == 'enum'):
                cur_i += 1
                cur_i = parseEnum(tokens, cur_i, cpp)
            elif tokens[cur_i] == '###':
                break
            else:
                cur_i = parseStruct(tokens, cur_i, cpp)
        f.close()

 
parseDocFiles(sys.argv[1:])
cpp.definitions += '''
std::string serializeAbility(const Ability& field) {
    return serializeAbility(field, 0);
}
'''

gencpp = open('gen_serializer.cpp', 'wt')
gencpp.write(cpp.headers)
gencpp.write('\n\n')
gencpp.write(cpp.declarations)
gencpp.write('\n\n')
gencpp.write(getVariantCode(cpp.variantCode))
gencpp.write('\n\n')
gencpp.write(cpp.definitions)
gencpp.close()

genhpp = open('gen_serializer.h', 'wt')
genhpp.write(cpp.hpp)
genhpp.close()
