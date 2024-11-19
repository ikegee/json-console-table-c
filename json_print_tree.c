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
gcc -o xyz .\json_print_tree.c cJSON.c -I.
.\xyz .\yourfile.json // no default file
*/

#define MAX_FILE_SIZE (10 * 10 * 1024) // 100KB
#define MIN_FILE_SIZE 2

bool is_json_file(FILE *file) {
    char first_char;
    if (!file) return false;
    
    first_char = fgetc(file);
    ungetc(first_char, file);
    
    // Check if file starts with { or [ which indicates JSON
    return (first_char == '{' || first_char == '[');
}

void print_json_array_tree(cJSON *item, int depth) {
    char prefix[256] = {0};
    for (int i = 0; i < depth; i++) {
        strcat(prefix, "  ");
    }

    if (item->string) {
        printf("%s|__ %s", prefix, item->string); // graphics component
    }

    switch (item->type) {
        case cJSON_Array:
            printf(" [Array]:\n");
            for (cJSON *child = item->child; child; child = child->next) {
                printf("%s    ", prefix);
                if (cJSON_IsNumber(child)) {
                    printf("%g\n", child->valuedouble);
                } else {
                    print_json_array_tree(child, depth + 1);
                }
            }
            break;
            
        case cJSON_Object:
            printf(" {Object}:\n");
            for (cJSON *child = item->child; child; child = child->next) {
                print_json_array_tree(child, depth + 1);
            }
            break;
            
        case cJSON_String:
            printf(": \"%s\"\n", item->valuestring);
            break;
            
        case cJSON_Number:
            printf(": %g\n", item->valuedouble);
            break;
            
        case cJSON_NULL:
            printf(": null\n");
            break;
            
        default:
            printf("\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <json_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file || !is_json_file(file)) {
        printf("Unable to open file: %s\n", argv[1]);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size > MAX_FILE_SIZE || file_size < MIN_FILE_SIZE) {
        printf("Invalid file size: %ld bytes\n", file_size);
        fclose(file);
        return 1;
    }

    char *json_string = malloc(file_size + 1);
    if (!json_string) {
        printf("Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    size_t bytes_read = fread(json_string, 1, file_size, file);
    json_string[bytes_read] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(json_string);
    if (!json) {
        const char *error_ptr = cJSON_GetErrorPtr();
        printf("JSON parsing error near: %.20s\n", error_ptr ? error_ptr : "unknown");
        free(json_string);
        return 1;
    }

    printf("\nJSON Array Structure for: %s\n", argv[1]);
    printf("---------------------------------------\n");
    print_json_array_tree(json, 0);
    printf("\n");

    cJSON_Delete(json);
    free(json_string);
    return 0;
}
