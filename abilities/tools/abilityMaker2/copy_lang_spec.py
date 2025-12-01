import sys


def readFileWithoutComments(f):
    doc = ''
    for line in f:
        if line.startswith('###'):
            break
        splitlines = line.split('//')
        doc += splitlines[0] + '\n'
    return doc

file_content = '''
#pragma once

#include <string>

const std::string kLangSpec = R"(
'''
for filename in sys.argv[1:]:
    f = open(filename, 'r')
    doc = readFileWithoutComments(f)
    f.close()

    file_content += doc
    file_content += '\n'

file_content += '''
)";
'''

genhpp = open('lang_spec.h', 'wt')
genhpp.write(file_content)
genhpp.close()