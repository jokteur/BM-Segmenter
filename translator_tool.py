"""
Run this file at the root of the git repo to see what is left
to translate or what has been added in the source files.
"""
from gettext import translation
import os
import re
import dataclasses
from typing import OrderedDict


def find_definitions():
    definitions = OrderedDict()
    for root, subFolders, files in os.walk("src"):
        for file in files:
            if file == "translate.h":
                pass
            else:
                path = os.path.join(root, file)
                with open(path) as f:
                    definitions[path] = []

                    content = "".join(f.readlines())
                    iter = re.finditer('TXT\(', content)

                    for match in iter:
                        idx_start = match.span()[1]
                        idx_end = idx_start
                        parenthesis_count = 1
                        count = True
                        while parenthesis_count > 0:
                            char = content[idx_end]
                            if char == '"':
                                count = not count
                            elif char == "(" and count:
                                parenthesis_count += 1
                            elif char == ")" and count:
                                parenthesis_count -= 1
                            if parenthesis_count == 1 and char == ",":
                                idx_end += 1
                                break
                            idx_end += 1
                        key = content[idx_start:idx_end  - 1]
                        definitions[path].append(key)

                    if definitions[path] == []:
                        del definitions[path]
    return definitions

def update_translation_file(path):
    already_translated = OrderedDict()
    
    definitions = find_definitions()

    with open(path) as f:
        content = "".join(f.readlines())
        flines = content.split("\n")

        fct_find = content.find("Translator build_")
        fct_start = 0
        if fct_find:
            fct_start = content.count("\n", 0, fct_find) + 3

        var_name = next(re.finditer("Translator ([a-zA-Z_]+);", content)).group(1)

        iter = re.finditer('texts\[', content)

        for match in iter:
            idx_start = match.span()[1]
            idx_end = idx_start
            bracket_count = 1
            count = True
            while bracket_count > 0:
                char = content[idx_end]
                if char == '"':
                    count = not count
                elif char == "[" and count:
                    bracket_count += 1
                elif char == "]" and count:
                    bracket_count -= 1
                idx_end += 1
            
            key = content[idx_start:idx_end - 1]
            already_translated[key] = next(re.finditer(f"{key}(.*)\n", content)).group(1)
            

        line_number = fct_start
        new_lines = []
        for file, value in definitions.items():
            new_lines.append("\n")
            file_line = line_number
            
            file = file.replace("\\", "/")
            new_lines.insert(file_line, f"    /* File: {file} */")

            for definition in value:
                if definition in already_translated:
                    new_lines.append(f"    {var_name}.texts[{definition}{already_translated[definition]}")
                else:
                    new_lines.append(f'    //{var_name}.texts[{definition}] = ""')

        return "\n".join(new_lines)


file = update_translation_file('src/ui/translations/french.cpp')
print(file)