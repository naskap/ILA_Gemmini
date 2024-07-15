import re
extract_varname_and_value = re.compile("  val (\w*) = ([\w(.)]*)")
extract_bitlength = re.compile('Bits\((\d*)\.W\)')

print("Input the fields declarations (newline when done): ")
user_input = []
while True:
    line = input()
    if line:
        user_input.append(line)
    else:
        break

for line in user_input:
    
    m = extract_varname_and_value.match(line)

    if m is None:
        continue

    varname = m.group(1)
    value = m.group(2)
    if("Bool" in value):
        value = 1
    else:
        assert "Bits" in value
        value = extract_bitlength.match(value).groups(1)[0]


    print(f"#define {varname.upper()}_WIDTH {value}")
