# Console table generator for JSON data structures using cJSON

A lightweight C utility that generates formatted console tables and tree views from JSON data structures. Built on top of the cJSON library, this tool provides clean and organized visualization of JSON data directly in the terminal.

## Example

Input JSON (json_example.json):
```json
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "compilerPath": "C:\\MinGW-w64\\bin\\gcc.exe",
            "cStandard": "c17",
            "cppStandard": "gnu++14",
            "intelliSenseMode": "windows-gcc-x64"
        }
    ],
    "version": 4
}
```
## Usage
- ## Usage
    Please see <span style="background-color:#FFFF00">`json_print_example.txt`</span>

- **Two Display Modes**:
  - Table View (`json_print_table.c`): Displays JSON arrays as formatted tables
  - ![json_print_table](screenshots/json_print_table.png)
  - Tree View (`json_print_tree.c`): Shows hierarchical JSON structure
  - ![json_print_tree](screenshots/json_print_tree.png)

- **Table View Features**:
  - Dynamic column width adjustment
  - Smart data type formatting (currency, numbers, text)
  - Row count limits for large datasets
  - Truncation of long text fields
  - Column filters and formatting rules
  - Support for nested JSON structures
  
- **Tree View Features**:
  - Visual hierarchy with indentation
  - Type indicators (Array, Object, String, Number)
  - Clear parent-child relationship display
  
## Requirements
 - C compiler (gcc recommended)
 - cJSON library (included)
 - Standard C libraries

## Limitations (Modifiable)
 - Maximum file size: 100KB
 - Maximum field width: 30 characters
 - Maximum displayed fields: 10
 - Maximum displayed rows: 20

## License
 - This project is licensed under the MIT License. See the LICENSE file for details.
 - cJSON library is Copyright (c) 2009-2017 Dave Gamble and cJSON contributors.
