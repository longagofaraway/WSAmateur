import sys
from io import StringIO


class Language:
    mode = ''
    def getLanguageComponentType(self):
        if self.mode == 'Trigger':
            return 'TriggerType'
        if self.mode == 'Effect':
            return 'EffectType'
        if self.mode == 'Condition':
            return 'ConditionType'
        if self.mode == 'Multiplier':
            return 'MultiplierType'
        raise 'unknown Language mode'
    

    def _getCurrentClassName(self):
        if self.mode == 'Trigger':
            return 'TriggerHelper'
        if self.mode == 'Effect':
            return 'EffectHelper'
        if self.mode == 'Condition':
            return 'ConditionHelper'
        if self.mode == 'Multiplier':
            return 'MultiplierHelper'
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
        if self.mode == 'Condition':
            return 'setConditionInQml(asn::ConditionType languageComponentType, const VarCondition& languageComponent)'
        if self.mode == 'Multiplier':
            return 'setMultiplierInQml(asn::MultiplierType languageComponentType, const VarMultiplier& languageComponent)'


    def _arraySizeGetterDeclaration(self):
        if self.mode == 'Trigger':
            return 'getArraySize(asn::TriggerType languageComponentType, const VarTrigger& languageComponent, QString field_name)'
        if self.mode == 'Effect':
            return 'getArraySize(asn::EffectType languageComponentType, const VarEffect& languageComponent, QString field_name)'
        if self.mode == 'Condition':
            return 'getArraySize(asn::ConditionType languageComponentType, const VarCondition& languageComponent, QString field_name)'
        if self.mode == 'Multiplier':
            return 'getArraySize(asn::MultiplierType languageComponentType, const VarMultiplier& languageComponent, QString field_name)'


    def getQmlSetterCode(self):
        class_name = self._getCurrentClassName()
        return f'''void {class_name}::{self._qmlSetterDeclaration()} {{
    switch (languageComponentType) {{
'''


    def getArraySizeGetterCode(self):
        class_name = self._getCurrentClassName()
        return f'''size_t {class_name}::{self._arraySizeGetterDeclaration()} {{
    switch (languageComponentType) {{
'''


    def getQmlSetterDeclaraion(self):
        return f'''void {self._qmlSetterDeclaration()};
'''

 
    def getArraySizeGetterDeclaraion(self):
        return f'''size_t {self._arraySizeGetterDeclaration()};
'''
    

    def _getArrayElementAdderSignature(self):
        return f'addElementToArray(QString field_name)'
    def _getArrayElementRemoverSignature(self):
        return f'removeElementFromArray(QString field_name)'


    def getArrayElementAdderDeclaration(self):
        return f'''void {self._getArrayElementAdderSignature()} override;
'''
    def getArrayElementRemoverDeclaration(self):
        return f'''void {self._getArrayElementRemoverSignature()} override;
'''
    

    def getBaseClassArrayElementAdderDefinition(self):
        return f'''virtual void {self._getArrayElementAdderSignature()} {{}}
'''
    def getBaseClassArrayElementRemoverDefinition(self):
        return f'''virtual void {self._getArrayElementRemoverSignature()} {{}}
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
        elif type == 'Condition':
            inputCppType = 'const asn::Condition&'
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
        else:
            inputCppType = 'QString'
        return f'{ltype}Changed({inputCppType} value, QString componentId)'


    def _getFunctionStarter(self, function_declaration, additional_code=''):
        class_name = self._getCurrentClassName()
        if self.mode == 'Trigger':
            component_type_type = 'asn::TriggerType'
            component_type = 'VarTrigger'
        elif self.mode == 'Effect':
            component_type_type = 'asn::EffectType'
            component_type = 'VarEffect'
        elif self.mode == 'Condition':
            component_type_type = 'asn::ConditionType'
            component_type = 'VarCondition'
        elif self.mode == 'Multiplier':
            component_type_type = 'asn::MultiplierType'
            component_type = 'VarMultiplier'
        return f'''void {class_name}::{function_declaration} {{
    auto languageComponentType = linkObject->getLanguageComponentType(formats::To<{component_type_type}>{{}});
    auto& languageComponent = linkObject->getLanguageComponent(formats::To<{component_type}>{{}});
    {additional_code}
    switch (languageComponentType) {{
'''


    def getSlotFunctionDefinition(self, type):
        idParser = 'auto [typePosition, arrayPosition] = parseComponentId(componentId);'
        return self._getFunctionStarter(self._slotFunctionDeclaration(type), idParser)
    

    def getArrayElementAdderDefinition(self):
        return self._getFunctionStarter(self._getArrayElementAdderSignature())
    def getArrayElementRemoverDefinition(self):
        return self._getFunctionStarter(self._getArrayElementRemoverSignature())


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


    def getArraySizeGetterCodeFooter(self):
        return '''    }
    return 1; 
}
'''


    def getSlotCodeFooter(self, type):
        return f'''    default:
        qWarning() << "unhandled {type}";
        throw std::logic_error("unhandled {type}");
    }}
    linkObject->notifyOfChanges();
}}
'''


class ParsedStruct:
    def __init__(self):
        self.qml_setter_code = ''
        self.slot_code = {}
        self.array_size_getter_code = ''
        self.array_appender_code = ''
        self.array_remover_code = ''


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
    array_size_getter_code = signal_code
    array_appender_code = signal_code
    array_remover_code = signal_code
    type_matches = {}
    type_position = 0
    slot_code = {}
    for line in ss:
        braces_count += countBracesInLine(line)
        if braces_count == 0:
            break
        tokens = prepareTokens(line)
        if len(tokens) < 2:
            continue

        type_position += 1
        field_name = tokens[0]
        is_array = False
        is_optional = False
        if tokens[1] == 'Array':
            field_type = tokens[3]
            is_array = True
            array_size_getter_code += f'''        if (field_name.toLower() == "{field_name.lower()}") {{
'''
            array_appender_code += f'''        if (field_name.toLower() == "{field_name.lower()}") {{
'''
            array_remover_code += f'''        if (field_name.toLower() == "{field_name.lower()}") {{
'''
        elif tokens[1] == 'Choice':
            field_type = parseChoice(ss, current_line)
            is_optional = True
            braces_count -= 1
        else:
            field_type = tokens[1]

        type_matches[field_type] = type_matches.get(field_type, 0) + 1
        if field_type in ['Bool', 'Target', 'Card', 'Multiplier', 'TargetAndPlace', 'Number', 'Place', 'Duration', 'Ability', 'SearchTarget', 'EventAbility', 'Effect', 'ChooseCard', 'AutoAbility', 'Condition']:
            prepared_signal_field = f'elem.{field_name}'
            prepared_slot_field = 'value'
            if is_optional:
                prepared_signal_field += '.value()'
            if ((struct_name in ['DelayedAbility', 'CostSubstitution'] and field_type in ['AutoAbility', 'Effect']) or
                struct_name in ['ForEachMultiplier', 'AddLevelMultiplier', 'AddTriggerNumberMultiplier'] and field_type == 'Target'):
                prepared_signal_field = '*'+prepared_signal_field+'.get()'
                prepared_slot_field = f'std::make_shared<asn::{field_type}>('+prepared_slot_field+')'
            if is_array:
                prepared_array_size_getter_field = prepared_signal_field + '.size()'
                prepared_array_appender = prepared_signal_field + f'.push_back(defaultConstructor.get{field_type}())'
                prepared_array_remover = prepared_signal_field + '.pop_back()'
                prepared_signal_field_last_slot = prepared_signal_field + f'[{prepared_array_size_getter_field}-1]'
                prepared_signal_field += '[i]'
        elif field_type in ['Int32', 'UInt8', 'Int8']:
            prepared_signal_field = f'QString::number(elem.{field_name})'
            prepared_slot_field = f'value.toInt()'
        elif field_type == 'String':
            prepared_signal_field = f'QString::fromStdString(elem.{field_name})'
            if is_array:
                prepared_signal_field = f'QString::fromStdString(elem.{field_name}[i])'
                prepared_array_size_getter_field = f'elem.{field_name}.size()'
                prepared_signal_field_last_slot = f'QString::fromStdString(elem.{field_name}[{prepared_array_size_getter_field}-1])'
                prepared_array_appender = f'elem.{field_name}.push_back("")'
                prepared_array_remover = f'elem.{field_name}.pop_back()'
            prepared_slot_field = 'value.toStdString()' 
        else: #enum
            prepared_signal_field = f'QString::fromStdString(toString(elem.{field_name}))'
            if is_array:
                prepared_signal_field = f'QString::fromStdString(toString(elem.{field_name}[i]))'
                prepared_array_size_getter_field = f'elem.{field_name}.size()'
                prepared_signal_field_last_slot = f'QString::fromStdString(toString(elem.{field_name}[{prepared_array_size_getter_field}-1]))'
                prepared_array_appender = f'elem.{field_name}.push_back(elem.{field_name}.back())'
                prepared_array_remover = f'elem.{field_name}.pop_back()'
            prepared_slot_field = f'parse(value.toStdString(), formats::To<asn::{field_type}>{{}})'
        component_id = field_type + '/' + str(type_position) + '/'
        array_index = '0'
        slot_type_position_check = f'if (typePosition == "{type_position}")'
        if is_array:
            signal_code += f'''        for (int i = 0; i < elem.{field_name}.size(); ++i) {{
    '''
            array_index = 'i'
        if is_optional:
            signal_code += f'''        if (elem.{field_name}.has_value()) {{
    '''
        signal_code += f'''        emit linkObject->set{field_type}({prepared_signal_field}, "{component_id}"+QString::number({array_index}));
'''
        if is_optional:
            signal_code += '''        }
'''
        if is_array:
            signal_code += '''        }
'''
            slot_code[field_type] = slot_code.get(field_type, '') + f'''        {slot_type_position_check} {{
            if (elem.{field_name}.size() <= arrayPosition.toInt()) {{
                qWarning() << "wrong arrat index for {field_name}";
                throw std::runtime_error("wrong arrat index for {field_name}");
            }}
            elem.{field_name}[arrayPosition.toInt()] = {prepared_slot_field};
        }}
'''
            array_size_getter_code += f'''            return {prepared_array_size_getter_field};
        }}
'''
            array_appender_code += f'''            {prepared_array_appender};
            emit linkObject->set{field_type}({prepared_signal_field_last_slot}, "{component_id}"+QString::number({prepared_array_size_getter_field}-1));
        }}
'''
            array_remover_code += f'''            {prepared_array_remover};
        }}
'''
        else:
            slot_code[field_type] = slot_code.get(field_type, '') + f'''        {slot_type_position_check} {{
            elem.{field_name} = {prepared_slot_field};
        }}
'''
                

    signal_code += '''        break;
    }
'''
    array_size_getter_code += '''        break;
    }
'''
    array_appender_code += '''        break;
    }
'''
    array_remover_code += '''        break;
    }
'''
    result = ParsedStruct()
    result.qml_setter_code = signal_code
    result.array_size_getter_code = array_size_getter_code
    result.array_appender_code = array_appender_code
    result.array_remover_code = array_remover_code
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
#include "asnTypeConstructor.h"
#include "baseComponent.h"

namespace gen {

using VarTrigger = decltype(asn::Trigger::trigger);
using VarEffect = decltype(asn::Effect::effect);
using VarCondition = decltype(asn::Condition::cond);
using VarMultiplier = decltype(asn::Multiplier::specifier);

class ComponentMediator : public QObject {
    Q_OBJECT
protected:
    BaseComponent *linkObject;
    AsnTypeConstructor defaultConstructor;
public:
    ComponentMediator(BaseComponent *linkObject) : linkObject(linkObject) {}

public slots:
'''

cpp = '''// This file is autogenerated. Do not edit
#include "ability_maker_gen.h"

#include "language_parser.h"
#include "componentIdParser.h"

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
    array_size_getter_code = ''
    array_appender_code = ''
    array_remover_code = ''
    for line in doc_ss:
        tokens = line.split()
        if len(tokens) < 1:
            continue
        if tokens[0] == 'Trigger' or tokens[0] == 'Effect'or tokens[0] == 'Condition' or tokens[0] == 'Multiplier':
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
        array_size_getter_code += parse_result.array_size_getter_code
        array_appender_code += parse_result.array_appender_code
        array_remover_code += parse_result.array_remover_code

    qml_setter_code = lang.getQmlSetterCode() + signal_code
    array_size_getter_code = lang.getArraySizeGetterCode() + array_size_getter_code
    array_appender_code = lang.getArrayElementAdderDefinition() + array_appender_code
    array_remover_code = lang.getArrayElementRemoverDefinition() + array_remover_code
    hpp_buffer += lang.getQmlSetterDeclaraion()
    hpp_buffer += lang.getArraySizeGetterDeclaraion()
    hpp_buffer += lang.getArrayElementAdderDeclaration()
    hpp_buffer += lang.getArrayElementRemoverDeclaration()

    qml_setter_code += lang.getQmlSetterCodeFooter()
    array_size_getter_code += lang.getArraySizeGetterCodeFooter()
    array_appender_code += lang.getSlotCodeFooter('')
    array_remover_code += lang.getSlotCodeFooter('')
    cpp += qml_setter_code
    cpp += array_size_getter_code
    cpp += array_appender_code
    cpp += array_remover_code
    for key in slot_code:
        all_types.update([key])
        hpp_buffer += lang.getSlotFunctionDeclaraion(key)
        slot_function = lang.getSlotFunctionDefinition(key) + slot_code[key] + lang.getSlotCodeFooter(key)
        cpp += slot_function
    hpp_buffer += '''
};

'''

hpp += global_lang.getBaseClassArrayElementAdderDefinition()
hpp += global_lang.getBaseClassArrayElementRemoverDefinition()
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