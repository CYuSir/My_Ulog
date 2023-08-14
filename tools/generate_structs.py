import os
import re

# Specify the correct paths for your input and output files
input_file = "structs_definitions.hpp"
output_file = "escape_structs.hpp"

# Get the current directory
current_dir = os.path.dirname(os.path.abspath(__file__))

# Construct full paths for input and output files
input_file_path = os.path.join(current_dir, input_file)
output_file_path = os.path.join(current_dir, output_file)

# Extract struct definitions from input file
struct_definitions = []
with open(input_file_path, 'r') as input_file:
    file_content = input_file.read()
    struct_matches = re.findall(r'struct\s+(\w+)\s*{([^}]+)};', file_content, re.DOTALL)
    for struct_name, struct_content in struct_matches:
        struct_definitions.append((struct_name.strip(), struct_content.strip()))

# Generate output file content with static methods
output_file_content = f'#pragma once\n\n#include <cstdint>\n#include <string>\n#include <vector>\n#include <variant>\n#include <ulog_cpp/logger.hpp>\n\n'

# Loop through struct definitions
for struct_name, struct_content in struct_definitions:
    output_file_content += f'struct {struct_name} {{\n'
    output_file_content += f'    {struct_content}\n\n'
    output_file_content += f'    static std::string messageName()  {{ return "{struct_name}"; }}\n\n'
    output_file_content += '    static std::vector<ulog_cpp::Field> fields()  {\n'
    output_file_content += '        // clang-format off\n'

    # Extract field names, types, and array sizes from struct_content
    field_matches = re.findall(r'(\w+)\s+(\w+)\s*(?:\[(\d+)\])?\s*;', struct_content)

    # Calculate field type size and add to field_matches as tuple (field_type, field_name, array_size, type_size)
    # Define field type size dictionary
    field_size = {
        "int8_t": 1,
        "uint8_t": 1,
        "int16_t": 2,
        "uint16_t": 2,
        "int32_t": 4,
        "uint32_t": 4,
        "int64_t": 8,
        "uint64_t": 8,
        "float": 4,
        "double": 8,
        "bool": 1,
        "char": 1,
        # Add more types and their sizes if needed
    }
    field_matches_with_size = [
        (field_type, field_name, array_size, field_size.get(field_type, 0))
        for field_type, field_name, array_size in field_matches
    ]

    # Sort fields based on data type size in descending order
    field_matches_with_size.sort(key=lambda x: x[3], reverse=True)

    output_file_content += '        return {\n'
    for field_type, field_name, array_size, _ in field_matches_with_size:
        if array_size:
            output_file_content += f'            {{"{field_type}", "{field_name}", {array_size}}},\n'
        else:
            output_file_content += f'            {{"{field_type}", "{field_name}"}},\n'

    output_file_content += '        };\n'
    output_file_content += '        // clang-format on\n'
    output_file_content += '    }\n'
    output_file_content += '};\n\n'



# Generate vector containing instances of each struct
output_file_content += '// Disable editing of DataVariant variable names\n'
output_file_content += 'using DataVariant = std::variant<' + ', '.join(struct_name for struct_name, _ in struct_definitions) + '>;\n\n'
output_file_content += 'static std::vector<DataVariant> all_structs = {\n'
for struct_name, _ in struct_definitions:
    output_file_content += f'    {struct_name}{{}},\n'
output_file_content += '};\n\n'

# Write the extracted struct definitions and vector
with open(output_file_path, 'w') as output_file:
    output_file.write(output_file_content)

print(f"Generated C++ code written to {output_file_path}")
