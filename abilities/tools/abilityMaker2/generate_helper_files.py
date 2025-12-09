import sys
from io import StringIO


class Language:
    mode = ''
    def getLanguageComponentType(self):
        if self.mode == 'Trigger':
            return 'TriggerType'
        if self.mode == 'Effect':
            return 'EffectType'
        raise 'unknown Language mode'
    

    def _getCurrentClassName(self):
        if self.mode == 'Trigger':
            return 'TriggerHelper'
        elif self.mode == 'Effect':
            return 'EffectHelper'
        raise 'unknown Language mode'


    def getClassDeclaration(self):
        class_name = self._getCurrentClassName()
        return f'''class {class_name} : public ComponentMediator {{
    Q_OBJECT
public:
    {class_name}(BaseComponent *linkObject) : ComponentMediator(linkObject) {{}}

public slots:
'''


    def _qmlSetterDeclaration(self):
        if self.mode == 'Trigger':
            return 'setTriggerInQml(asn::TriggerType languageComponentType, const VarTrigger& languageComponent)'
        if self.mode == 'Effect':
            return 'setEffectInQml(asn::EffectType languageComponentType, const VarEffect& languageComponent)'


    def getQmlSetterCode(self):
        class_name = self._getCurrentClassName()
        return f'''void {class_name}::{self._qmlSetterDeclaration()} {{
    switch (languageComponentType) {{
'''


    def getQmlSetterDeclaraion(self):
        return f'''void {self._qmlSetterDeclaration()};
'''


    def _slotFunctionDeclaration(self, type):
        ltype = type[0].lower() + type[1:]
        if type == 'Bool':
            inputCppType = 'bool'
        elif type == 'Target':
            inputCppType = 'const asn::Target&'
        elif type == 'Card':
            inputCppType = 'const asn::Card&'
        elif type == 'TargetAndPlace':
            inputCppType = 'const asn::TargetAndPlace&'
        elif type == 'SearchTarget':
            inputCppType = 'const asn::SearchTarget&'
        elif type == 'Effect':
            inputCppType = 'const asn::Effect&'
        elif type == 'EventAbility':
            inputCppType = 'const asn::EventAbility&'
        elif type == 'AutoAbility':
            inputCppType = 'const asn::AutoAbility&'
        elif type == 'Ability':
            inputCppType = 'const asn::Ability&'
        elif type == 'ChooseCard':
            inputCppType = 'const asn::ChooseCard&'
        elif type == 'Number':
            inputCppType = 'const asn::Number&'
        elif type == 'Multiplier':
            inputCppType = 'const asn::Multiplier&'
        elif type == 'Place':
            inputCppType = 'const asn::Place&'
        elif type == 'Duration':
            inputCppType = 'int'
        elif type in ['UInt8', 'Int32', 'Int8']:
            inputCppType = 'int'
        else:
            inputCppType = 'QString'
        return f'{ltype}Changed({inputCppType} value, QString componentId)'


    def getSlotFunctionDefinition(self, type):
        class_name = self._getCurrentClassName()
        if self.mode == 'Trigger':
            component_type_type = 'asn::TriggerType'
            component_type = 'VarTrigger'
        elif self.mode == 'Effect':
            component_type_type = 'asn::EffectType'
            component_type = 'VarEffect'
        return f'''void {class_name}::{self._slotFunctionDeclaration(type)} {{
    auto languageComponentType = linkObject->getLanguageComponentType(formats::To<{component_type_type}>{{}});
    auto& languageComponent = linkObject->getLanguageComponent(formats::To<{component_type}>{{}});
    switch (languageComponentType) {{
'''


    def getBaseClassSlotDefinition(self, type):
        return f'''virtual void {self._slotFunctionDeclaration(type)} {{}}
'''


    def getSlotFunctionDeclaraion(self, type):
        return f'''void {self._slotFunctionDeclaration(type)} override;
'''


    def getQmlSetterCodeFooter(self):
        return '''    }
}
'''


    def getSlotCodeFooter(self, type):
        return f'''    default:
        throw std::logic_error("unhandled {type}");
    }}
    linkObject->notifyOfChanges();
}}
'''


class ParsedStruct:
    def __init__(self):
        self.qml_setter_code = ''
        self.slot_code = {}


def readFileWithoutComments(f):
    doc = ''
    for line in f:
        if line.startswith('###'):
            break
        splitlines = line.split('//')
        doc += splitlines[0] + '\n'
    return doc


def parseRelation(doc):
    brace_start = doc.find('{')
    if brace_start == -1:
        raise 'malformed relation'
    brace_end = doc.find('}', brace_start+1)
    if brace_end == -1:
        raise 'malformed relation'

    lines = doc[brace_start+1:brace_end].splitlines()
    relation = {}
    for line in lines:
        splitted = line.split(':')
        if len(splitted) < 2:
            continue
        enum_value = splitted[0].strip(' ,')
        enum_value = enum_value[0].upper() + enum_value[1:]
        struct_name = splitted[1].strip(' ,')
        relation[struct_name] = enum_value
    return relation


def countBracesInLine(line):
    braces_count = 0
    for char in line:
        if char == '{':
            braces_count = braces_count + 1
        elif char == '}':
            braces_count = braces_count - 1
    return braces_count


def prepareTokens(line):
    result_tokens = []
    tokens = line.split()
    for token in tokens:
        if len(token.strip()) == 0:
            continue
        if token.endswith(','):
            token = token[:-1]
        result_tokens.append(token)
    return result_tokens


def skipBlock(ss, current_line):
    braces_count = countBracesInLine(current_line)
    if braces_count == 0:
        return

    for line in ss:
        braces_count += countBracesInLine(line)
        if braces_count == 0:
            return


def parseChoice(ss, current_line):
    braces_count = countBracesInLine(current_line)
    if braces_count == 0:
        return
    
    choice_type = ''
    for line in ss:
        braces_count += countBracesInLine(line)
        if braces_count == 0:
            break
        line = line.strip()
        if line.endswith(','):
            line = line[:-1]
        if line == 'Nothing':
            continue
        if not line:
            continue
        choice_type = line
    return choice_type


def parseStruct(ss, current_line, lang):
    braces_count = countBracesInLine(current_line)
    if braces_count == 0:
        return

    tokens = current_line.split()
    struct_name = tokens[0]
    signal_code = f'''    case asn::{lang.getLanguageComponentType()}::{lang.relation[struct_name]}: {{
        auto &elem = std::get<asn::{struct_name}>(languageComponent);
'''
    slot_code_header = signal_code
    type_matches = {}
    slot_code = {}
    for line in ss:
        braces_count += countBracesInLine(line)
        if braces_count == 0:
            break
        tokens = prepareTokens(line)
        if len(tokens) < 2:
            continue

        field_name = tokens[0]
        is_array = False
        is_optional = False
        if tokens[1] == 'Array':
            field_type = tokens[3]
            is_array = True
        elif tokens[1] == 'Choice':
            field_type = parseChoice(ss, current_line)
            is_optional = True
            braces_count -= 1
        else:
            field_type = tokens[1]

        type_matches[field_type] = type_matches.get(field_type, 0) + 1
        if field_type in ['Bool', 'Target', 'Card', 'Int32', 'Int8', 'UInt8', 'Multiplier', 'TargetAndPlace', 'Number', 'Place', 'Duration', 'Ability', 'SearchTarget', 'EventAbility', 'Effect', 'ChooseCard', 'AutoAbility']:
            prepared_signal_field = f'elem.{field_name}'
            prepared_slot_field = 'value'
            if struct_name in ['DelayedAbility', 'CostSubstitution'] and field_type in ['AutoAbility', 'Effect']:
                prepared_signal_field = '*'+prepared_signal_field+'.get()'
                prepared_slot_field = f'std::make_shared<asn::{field_type}>('+prepared_slot_field+')'
            if is_array:
                prepared_signal_field += '[0]'
        elif field_type == 'String':
            prepared_signal_field = f'QString::fromStdString(elem.{field_name})'
            if is_array:
                prepared_signal_field = f'QString::fromStdString(elem.{field_name}[0])'
            prepared_slot_field = 'value.toStdString()' 
        else: #enum
            prepared_signal_field = f'QString::fromStdString(toString(elem.{field_name}))'
            if is_array:
                prepared_signal_field = f'QString::fromStdString(toString(elem.{field_name}[0]))'
            prepared_slot_field = f'parse(value.toStdString(), formats::To<asn::{field_type}>{{}})'
        component_id = field_type
        slot_repeat_check = 'if (componentId.back().digitValue() == -1)'
        if type_matches[field_type] > 1:
            component_id += str(type_matches[field_type])
            slot_repeat_check = f'if (componentId.back().digitValue() == {type_matches[field_type]})'
        if is_array:
            signal_code += f'''        if (elem.{field_name}.size() > 0) {{
    '''
        optional_getter = ''
        if is_optional:
            signal_code += f'''        if (elem.{field_name}.has_value()) {{
    '''
            optional_getter = '.value()'
        signal_code += f'''        emit linkObject->set{field_type}({prepared_signal_field}{optional_getter}, "{component_id}");
'''
        if is_optional:
            signal_code += '''        }
'''
        if is_array:
            signal_code += '''        }
'''
            slot_code[field_type] = slot_code.get(field_type, '') + f'''        {slot_repeat_check} {{
            elem.{field_name}.clear();
            elem.{field_name}.push_back({prepared_slot_field});
        }}
'''
        else:
            slot_code[field_type] = slot_code.get(field_type, '') + f'''        {slot_repeat_check} {{
            elem.{field_name} = {prepared_slot_field};
        }}
'''
                

    signal_code += '''        break;
    }
'''
    result = ParsedStruct()
    result.qml_setter_code = signal_code
    for key in slot_code:
        result.slot_code[key] = slot_code_header + slot_code[key] + '''        break;
    }
'''
    return result


hpp = '''// This file is autogenerated. Do not edit
#pragma once

#include <string>

#include <QObject>

#include "abilities.h"
#include "baseComponent.h"

namespace gen {

using VarTrigger = decltype(asn::Trigger::trigger);
using VarEffect = decltype(asn::Effect::effect);

class ComponentMediator : public QObject {
    Q_OBJECT
protected:
    BaseComponent *linkObject;
public:
    ComponentMediator(BaseComponent *linkObject) : linkObject(linkObject) {}

public slots:
'''

cpp = '''// This file is autogenerated. Do not edit
#include "ability_maker_gen.h"

#include "language_parser.h"

namespace gen {
'''

hpp_buffer = ''
all_types = set()
global_lang = Language()
for filename in sys.argv[1:]:
    f = open(filename, 'r')
    doc = readFileWithoutComments(f)
    f.close()

    pos = doc.find('relation')
    if pos == -1:
        continue
    lang = Language()
    lang.relation = parseRelation(doc[pos:])

    doc_ss = StringIO(doc)
    slot_code = {}
    signal_code = ''
    for line in doc_ss:
        tokens = line.split()
        if len(tokens) < 1:
            continue
        if tokens[0] == 'Trigger' or tokens[0] == 'Effect':
            skipBlock(doc_ss, line)
            lang.mode = tokens[0]
            hpp_buffer += lang.getClassDeclaration()
            continue
        if tokens[0] in ['relation','enum','TargetAndPlace', 'SearchTarget', 'PayCost']:
            skipBlock(doc_ss, line)
            continue

        parse_result = parseStruct(doc_ss, line, lang)
        signal_code += parse_result.qml_setter_code
        slot_code = {k: slot_code.get(k, '') + parse_result.slot_code.get(k, '') for k in slot_code.keys() | parse_result.slot_code.keys()}

    qml_setter_code = lang.getQmlSetterCode() + signal_code
    hpp_buffer += lang.getQmlSetterDeclaraion()

    qml_setter_code += lang.getQmlSetterCodeFooter()
    cpp += qml_setter_code
    for key in slot_code:
        all_types.update([key])
        hpp_buffer += lang.getSlotFunctionDeclaraion(key)
        slot_function = lang.getSlotFunctionDefinition(key) + slot_code[key] + lang.getSlotCodeFooter(key)
        cpp += slot_function
    hpp_buffer += '''
};

'''

for key in all_types:
    hpp += global_lang.getBaseClassSlotDefinition(key)
hpp += '''
};

'''

hpp += hpp_buffer
hpp += '''

}
'''
cpp += '''
}
'''

gencpp = open('ability_maker_gen.cpp', 'wt')
gencpp.write(cpp)
gencpp.close()

genhpp = open('ability_maker_gen.h', 'wt')
genhpp.write(hpp)
genhpp.close()