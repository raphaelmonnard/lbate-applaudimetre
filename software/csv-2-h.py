import csv
import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))

csv_file = "spectacle.csv"
header_files = [
    "./receiver/spectacle.h",
    "./transmitter/spectacle.h"
]

bool_array = []
int_array = []
string_array = []

# Read the CSV file
with open(csv_file, 'r') as file:
    reader = csv.reader(file)
    next(reader)  # Skip the header row
    
    for row in reader:
        int_array.append(row[3:9])
        string_array.append([row[1]])

# Generate the header file content
header_content = ""

# Generate the int table
header_content += "const int params[{}][{}] PROGMEM = {{\n".format(len(int_array), len(int_array[0]))
header_content += ",\n".join("    {" + ",".join(str(value) for value in row) + "}" for row in int_array)
header_content += "\n};\n\n"

# Generate the optimized desc table code
desc_lines = []
desc_array_elements = []

for index, row in enumerate(string_array):
    desc_var_name = f"desc_{index}"
    desc_value = row[0]
    desc_line = f'const char {desc_var_name}[] PROGMEM = "{desc_value}";'
    desc_lines.append(desc_line)
    desc_array_elements.append(desc_var_name)

desc_array = ",\n  ".join(desc_array_elements)
desc_array_code = f"\nconst char *const desc[] PROGMEM = {{\n  {desc_array}\n}};"

final_code = header_content + "\n".join(desc_lines + [desc_array_code])

# Write the header file
for file in header_files:
    with open(file, 'w') as file:
        file.write(final_code)

    print("Conversion complete. Header file '{}' has been generated.".format(file))