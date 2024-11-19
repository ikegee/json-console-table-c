/*
 * Copyright (c) 2024 G.E. Eidsness.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h" //Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

/** 
gcc -o xyz json_print_table.c cJSON.c -I. // -fno-syntax-only
.\xyz OR .\xyz .\yourfile.json 
*/

#define DEFAULT_JSON "json_example.json"
#define MAX_FILE_SIZE (10 * 10 * 1024) // 100 KB 
#define MIN_FILE_SIZE 2
#define ROW_COUNT 20
#define FIELD_COUNT 10
#define MIN_FIELD_WIDTH 5
#define MAX_FIELD_WIDTH 30
#define FORMAT_BOOL 0
#define FORMAT_DEFAULT 0
#define FORMAT_CURRENCY 1
#define FORMAT_AGE 3
#define FORMAT_ID 4
#define FORMAT_TITLE 20

typedef struct {
    char **names;
    int count;
    int *widths;
} TableHeaders;

typedef struct {
    char *pattern;
    int format_type;
} ColumnFilter;

static const ColumnFilter filters[] = {
    {"id", FORMAT_ID},
    {"text", MAX_FIELD_WIDTH},
    {"flag", FORMAT_BOOL},
    {"count", FORMAT_ID + 2},
    {"amount", FORMAT_CURRENCY},
    {"title", MAX_FIELD_WIDTH},
    {"wage", FORMAT_CURRENCY},
    {"price", FORMAT_CURRENCY},
    {"cost", FORMAT_CURRENCY},
    {"salary", FORMAT_CURRENCY},
    {"payment", FORMAT_CURRENCY},
    {"age", FORMAT_AGE},
    {"name", MAX_FIELD_WIDTH},
    {"description", MAX_FIELD_WIDTH},
    {NULL, FORMAT_DEFAULT}  // terminator
};

TableHeaders* extract_headers(cJSON *array_item) {
    TableHeaders *headers = malloc(sizeof(TableHeaders));
    headers->names = NULL;
    headers->count = 0;
    headers->widths = NULL;

    if (!cJSON_IsArray(array_item) || !cJSON_GetArraySize(array_item)) {
        return headers;
    }

    // Get first object to extract field names
    cJSON *first_item = cJSON_GetArrayItem(array_item, 0);
    cJSON *current_field = first_item->child;
    
    // Count fields
    while (current_field) {
        headers->count++;
        current_field = current_field->next;
    }

    // Allocate memory for names and widths
    headers->names = malloc(headers->count * sizeof(char*));
    headers->widths = malloc(headers->count * sizeof(int));

    // Store field names
    current_field = first_item->child;
    int i = 0;
    while (current_field) {
        headers->names[i] = strdup(current_field->string);
        headers->widths[i] = strlen(current_field->string);
        if (headers->widths[i] < MIN_FIELD_WIDTH) {
            headers->widths[i] = MIN_FIELD_WIDTH;
        }
        i++;
        current_field = current_field->next;
    }

    return headers;
}

bool is_json_file(FILE *file) {
    char first_char;
    if (!file) return false;
    
    first_char = fgetc(file);
    ungetc(first_char, file);
    
    // Check if file starts with { or [ which indicates JSON
    return (first_char == '{' || first_char == '[');
}

int validate_json_structure(cJSON *array_item) {
    if (!array_item || !cJSON_IsArray(array_item)) {
        return 0;
    }
    
    // Get first object to establish field pattern
    cJSON *first_item = cJSON_GetArrayItem(array_item, 0);
    if (!first_item) return 0;
    
    int field_count = 0;
    cJSON *field = first_item->child;
    while (field) {
        field_count++;
        field = field->next;
    }
    
    // Validate all items have same field count
    cJSON *item;
    int index = 0;
    cJSON_ArrayForEach(item, array_item) {
        int current_fields = 0;
        field = item->child;
        while (field) {
            current_fields++;
            field = field->next;
        }
        if (current_fields != field_count) {
            printf("Field count mismatch at row %d: expected %d, got %d\n", 
                   index, field_count, current_fields);
            return 0;
        }
        index++;
    }
    return 1;
}

void truncate_string(char *str, int max_length) {
    size_t len = strlen(str);
    if (len > max_length) {
        memcpy(str + max_length - 3, "...", 3);
        str[max_length] = '\0';
    }
}

int get_column_format(const char *field_name) {
    for (int i = 0; filters[i].pattern != NULL; i++) {
        if (strstr(field_name, filters[i].pattern) != NULL) {
            return filters[i].format_type;
        }
    }
    return FORMAT_DEFAULT;
}

void calculate_column_widths(cJSON *array_item, TableHeaders *headers) {
    char buffer[MAX_FIELD_WIDTH + 1];

    cJSON *row;
    cJSON_ArrayForEach(row, array_item) {
        for (int i = 0; i < headers->count; i++) {
            cJSON *field = cJSON_GetObjectItem(row, headers->names[i]);
            int field_width = 0;
        
            if (field->type == cJSON_Array || field->type == cJSON_Object) {
                field_width = 5;  // "[obj]" length
            }
            else if (field->type == cJSON_String) {
                field_width = strlen(field->valuestring);
            }
            if (field->type == cJSON_Number) {
                char num_str[MAX_FIELD_WIDTH + 1];
                snprintf(num_str, MAX_FIELD_WIDTH, "%g", field->valuedouble);
                field_width = strlen(num_str);
            
                if (get_column_format(headers->names[i]) == FORMAT_CURRENCY) {
                    field_width += 4; // Add space for "$", decimal places, and padding
                }
            }
            if (field_width > headers->widths[i]) {
                headers->widths[i] = field_width;
            }
        }
    }
}
 
void print_formatted_line(TableHeaders *headers, int fields_to_show) {
    for (int i = 0; i < fields_to_show; i++) {
        printf("+");
        for (int j = 0; j < headers->widths[i] + 2; j++) {
            printf("-");
        }
    }
    printf("+\n");
}

const char* get_table_name(cJSON *json) {
    // Update to directly return array name
    if (json && json->string) {
        return json->string;
    }
    return "Undefined";
}

void print_dynamic_table(cJSON *array_item, TableHeaders *headers) {
    int table_count = cJSON_GetArraySize(array_item); // Get actual row count
    char buffer[MAX_FIELD_WIDTH + 1];
    int row_count = 0;
    
    // Calculate total width for right alignment
    int total_width = 0;
    int fields_to_show = (headers->count > FIELD_COUNT) ? FIELD_COUNT : headers->count;
    for (int i = 0; i < fields_to_show; i++) {
        total_width += headers->widths[i] + 3;
    }
    
    // Print table name and right-aligned row count/limit on same line
    printf("\nTable: %s%*s%d/%d\n", 
           get_table_name(array_item),
           total_width - strlen(get_table_name(array_item)) - 10, 
           "", table_count, ROW_COUNT);

    // Print header row
    print_formatted_line(headers, fields_to_show);
    for (int i = 0; i < fields_to_show; i++) {
        printf("| %-*s ", headers->widths[i], headers->names[i]);
    }
    printf("|\n");
    print_formatted_line(headers, fields_to_show);

    // Print data rows with limits
    cJSON *row;
    cJSON_ArrayForEach(row, array_item) {
        if (row_count >= ROW_COUNT) break;
        
        for (int i = 0; i < fields_to_show; i++) {
            cJSON *field = cJSON_GetObjectItem(row, headers->names[i]);
            // Handle different data types
            if (field->type == cJSON_NULL) {
                printf("| %-*s ", headers->widths[i], "null");
            }
            else if (field->type == cJSON_String) {
                strncpy(buffer, field->valuestring, MAX_FIELD_WIDTH);
                buffer[MAX_FIELD_WIDTH] = '\0';
                truncate_string(buffer, MAX_FIELD_WIDTH-1);
                printf("| %-*s ", headers->widths[i], buffer);
            }
            else if (field->type == cJSON_Number) {
                // Try to detect if it's a special/scientific notation number
                char num_str[MAX_FIELD_WIDTH + 1];
                snprintf(num_str, MAX_FIELD_WIDTH, "%g", field->valuedouble);
                if (strchr(num_str, 'e') || strchr(num_str, 'E') || strlen(num_str) > MAX_FIELD_WIDTH) {
                    // Handle as string if it's scientific notation or very long
                    strncpy(buffer, num_str, MAX_FIELD_WIDTH);
                    buffer[MAX_FIELD_WIDTH] = '\0';
                    truncate_string(buffer, MAX_FIELD_WIDTH-1);
                    printf("| %-*s ", headers->widths[i], buffer);
                } else {
                    // Original number formatting logic here
                    if (get_column_format(headers->names[i]) == FORMAT_AGE) {
                        printf("| %*d ", headers->widths[i], field->valueint);
                    }
                    else if (field->valuedouble == (double)field->valueint && 
                            get_column_format(headers->names[i]) != FORMAT_CURRENCY) {
                        printf("| %*d ", headers->widths[i], field->valueint);
                    } else {
                        printf("| $%*.2f ", headers->widths[i] - 1, field->valuedouble);
                    }
                }
            }
            // Handle arrays/objects; notify user, no processing.
            else if (field->type == cJSON_Array || field->type == cJSON_Object) {
                printf("| %-*s ", headers->widths[i], "[obj]");
            }
        }
        printf("|\n");
        row_count++;
    }
    print_formatted_line(headers, fields_to_show);
}

void free_headers(TableHeaders *headers) {
    for (int i = 0; i < headers->count; i++) {
        free(headers->names[i]);
    }
    free(headers->names);
    free(headers->widths);
    free(headers);
}

int main(int argc, char *argv[]) {
    const char* jsonFile = DEFAULT_JSON;
    if (argc > 1) {
        jsonFile = argv[1];
    } else {
        jsonFile = DEFAULT_JSON;
    }

    FILE *file = fopen(jsonFile, "r");
    if (!file || !is_json_file(file)) {
        printf("Invalid file format: %s\n", jsonFile);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // File size validation
    if (file_size > MAX_FILE_SIZE) {
        printf("File too large: %ld bytes. Maximum allowed size is %d bytes\n", 
               file_size, MAX_FILE_SIZE);
        fclose(file);
        return 1;
    }
    
    if (file_size < MIN_FILE_SIZE) {
        printf("File is empty or too small to be valid JSON\n");
        fclose(file);
        return 1;
    }

    char *json_string = malloc(file_size + 1);
    if (!json_string) {
        printf("Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    // Read in chunks to handle large files reliably
    size_t total_read = 0;
    size_t bytes_read;
    while ((bytes_read = fread(json_string + total_read, 1, 
           file_size - total_read, file)) > 0) {
        total_read += bytes_read;
    }

    if (ferror(file)) {
        printf("Error occurred while reading file\n");
        free(json_string);
        fclose(file);
        return 1;
    }

    json_string[total_read] = '\0';
    fclose(file);

    // Parse JSON
    cJSON *json = cJSON_Parse(json_string);
    if (!json) {
        const char *error_ptr = cJSON_GetErrorPtr();
        int position = error_ptr - json_string;
        printf("JSON parsing error at position %d near: %.20s\n", 
            position, error_ptr ? error_ptr : "unknown");
        return 1;
    }

    // Check if root is direct array
    cJSON *current;    
    if (cJSON_IsArray(json)) {
        current = json;
    } else {
        // Otherwise use existing child iteration
        current = json->child;
    }

    // Iterate through the array
    while (current != NULL) {
        // Optimized nested check using current
        if (!current || !cJSON_IsArray(current)) {
            printf("JSON Format Unsupported; please adjust Code\n");
            printf("Current format: Nested objects\n");
            printf("Expected format: Array of objects\n\n");
            cJSON_Delete(json);
            free(json_string);
            return 1;
        }
        
        if (cJSON_IsArray(current)) {
            // Validate JSON structure
            if (!validate_json_structure(current)) {
                printf("Invalid JSON structure in array: %s\n", current->string);
                current = current->next;
                continue;
            }
            // ** Main Event: Extract headers and print table  ** //
            TableHeaders *headers = extract_headers(current);
            if (headers->count > 0) {
                calculate_column_widths(current, headers);
                print_dynamic_table(current, headers);
                // Add newline only if this is valid JSON (no warnings)
                if (current && cJSON_IsArray(current)) {
                    printf("\n");
                }
                free_headers(headers);
            }
        }
        current = current->next;
    }

    cJSON_Delete(json);
    free(json_string);
    return 0;
}
