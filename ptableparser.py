import json

# Load JSON data from file
with open("data.json", "r") as json_file:
    data = json_file.read()

# Parse JSON data
elements = json.loads(data)

# Define a function to format the output
def format_element(element):
    try:
        symbol = element.get("symbol", "Unknown")
        weight = element.get("weight", "Unknown")
        melt = element.get("melt", "Unknown")
        boil = element.get("boil", "Unknown")
        electroneg = element.get("electroneg", "Unknown")
        ionize_1 = element.get("ionize", {}).get("1", "Unknown")
        calculated_radius = element.get("radius", {}).get("calculated", "Unknown")
        return f'Element {symbol} = {{"{symbol}", {weight}, {melt}, {boil}, {electroneg}, {ionize_1}, {calculated_radius}}};'
    except Exception as e:
        print(f"Error formatting element {element.get('atomic', 'Unknown')}: {e}")
        return ""

# Output the elements in Arduino code format to a text file
with open("Parsedptabledata.txt", "w") as file:
    for element in elements:
        file.write(format_element(element) + "\n")
