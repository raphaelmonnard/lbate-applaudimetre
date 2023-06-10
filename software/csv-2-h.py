import csv

csv_file = "spectacle.csv"
header_file = "06_transmitter/spectacle.h"

bool_array = []
string_array = []

# Read the CSV file
with open(csv_file, 'r') as file:
    reader = csv.reader(file)
    next(reader)  # Skip the header row
    
    for row in reader:
        bool_array.append([value.lower() for value in row[3:]])
        string_array.append([row[1]])

# Generate the header file content
header_content = ""
header_content += "const bool stripes[{}][{}] = ".format(len(bool_array), len(bool_array[0]))
header_content += "{\n"
for row in bool_array:
    header_content += "    {"
    header_content += ", ".join(str(value) for value in row)
    header_content += "},\n"
header_content += "};\n\n"

# Generate the optimized desc table code
desc_lines = []
desc_array_elements = []

for index, row in enumerate(string_array):
    desc_var_name = f"desc_{index}"
    desc_value = row[0]
    desc_line = f'const char {desc_var_name}[] = "{desc_value}";'
    desc_lines.append(desc_line)
    desc_array_elements.append(desc_var_name)

desc_array = ",\n  ".join(desc_array_elements)
desc_array_code = f"const char* const desc[] = {{\n  {desc_array}\n}};"

final_code = header_content + "\n".join(desc_lines + [desc_array_code])

# Write the header file
with open(header_file, 'w') as file:
    file.write(final_code)

print("Conversion complete. Header file '{}' has been generated.".format(header_file))