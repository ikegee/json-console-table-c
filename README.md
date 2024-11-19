# Console table generator for JSON data structures using cJSON

A lightweight C utility that generates formatted console tables and tree views from JSON data structures. Built on top of the cJSON library, this tool provides clean and organized visualization of JSON data directly in the terminal.

## Features

- **Two Display Modes**:
  - Table View (`json_print_table.c`): Displays JSON arrays as formatted tables
  - Tree View (`json_print_tree.c`): Shows hierarchical JSON structure
  
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
  
## Building

```bash
gcc -o json_table json_print_table.c cJSON.c -I.
gcc -o json_tree json_print_tree.c cJSON.c -I.
