#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <log4c.h>
#include <yaml.h>

#include "config_mgr.h"
#include "../utils/mapping.h"
#include "../utils/helper.h"

int get_config_value(const char *config_file, const char *service, char* ip, int* port) {

    FILE *file = fopen(config_file, "r");
    if (!file) {
        perror("fopen");
        return -1;
    }

    yaml_parser_t parser;
    yaml_token_t token;
    bool found_service = false, found_ip = false, found_port = false;
    bool error = false;

    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize parser\n");
        fclose(file);
        return -1;
    }

    yaml_parser_set_input_file(&parser, file);

    // Do not search for ip and port if they are NULL
    if(ip == NULL) {
        found_ip = true;
    }
    if(port == NULL) {
        found_port = true;
    }

    while (!error && (!found_ip || !found_port)) {
        if (!yaml_parser_scan(&parser, &token)) {
            fprintf(stderr, "Parser error\n");
            error = true;
            break;
        }

        // If it is the end of the stream, break
        if (token.type == YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
            break;
        }

        if (token.type == YAML_KEY_TOKEN) {
            yaml_token_delete(&token);

            // Read the key value
            if (!yaml_parser_scan(&parser, &token)) {
                fprintf(stderr, "Parser error\n");
                error = true;
                break;
            }

            if (token.type == YAML_SCALAR_TOKEN) {
                if (!found_service && strcmp((char *)token.data.scalar.value, service) == 0) {
                    found_service = true;
                }
                else if (found_service && !found_ip && strcmp((char *)token.data.scalar.value, "ip") == 0) {
                    yaml_token_delete(&token);

                    // Read the token VALUE
                    if (!yaml_parser_scan(&parser, &token)) {
                        fprintf(stderr, "Parser error\n");
                        error = true;
                        break;
                    }
                    yaml_token_delete(&token);

                    // Read the actual value
                    if (!yaml_parser_scan(&parser, &token)) {
                        fprintf(stderr, "Parser error\n");
                        error = true;
                        break;
                    }

                    if (token.type == YAML_SCALAR_TOKEN) {
                        strcpy(ip, (char *)token.data.scalar.value);

                        found_ip = true; // ip found
                    }
                }
                else if (found_service && !found_port && strcmp((char *)token.data.scalar.value, "port") == 0) {
                    yaml_token_delete(&token);

                    // Read the token VALUE
                    if (!yaml_parser_scan(&parser, &token)) {
                        fprintf(stderr, "Parser error\n");
                        error = true;
                        break;
                    }
                    yaml_token_delete(&token);

                    // Read the actual value
                    if (!yaml_parser_scan(&parser, &token)) {
                        fprintf(stderr, "Parser error\n");
                        error = true;
                        break;
                    }

                    if (token.type == YAML_SCALAR_TOKEN) {
                        *port = atoi((char *)token.data.scalar.value);

                        found_port = true;  // Port found
                    }
                }
            }
        }

        yaml_token_delete(&token);
    }

    yaml_parser_delete(&parser);
    fclose(file);

    return error ? FAILURE : SUCCESS;
}

void set_default_log(char *level, char *pattern, char *filename, bool *enableFile, bool *enableConsole) {
    if(level != NULL && strlen(level) == 0) strcpy(level, "DEBUG");
    if(pattern != NULL && strlen(pattern) == 0) strcpy(pattern, "%d{%Y-%m-%d %H:%M:%S} %-5p %c - %m%n");
    if(filename != NULL && strlen(filename) == 0) strcpy(filename, "GAME_RULES_MGR.log");
    if(enableFile != NULL) *enableFile = true;
    if(enableConsole != NULL) *enableConsole = true;
}

int read_log_config(const char *config_file, log_levels *l_level, char *pattern, char *filename, bool *enableFile, bool *enableConsole) {
    FILE *file = fopen(config_file, "r");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    yaml_parser_t parser;
    memset(&parser, 0, sizeof(parser)); // Ensure parser is zero-initialized
    yaml_token_t token;

    char level[100];

    printf("Reading log config\n");

    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize parser\n");
        fclose(file);
        return -1;
    }

    printf("Parser initialized\n");

    yaml_parser_set_input_file(&parser, file);

    printf("Input file set\n");

    bool found_log = false;
    bool found_level = false;
    bool found_pattern = false;
    bool found_filename = false;
    bool found_enableFile = false;
    bool found_enableConsole = false;

    while (yaml_parser_scan(&parser, &token)) {
        if (token.type == YAML_NO_TOKEN) {
            fprintf(stderr, "Error: No token found. Possible parser error.\n");
            yaml_token_delete(&token);
            break;
        }

        if(token.type == YAML_STREAM_END_TOKEN) {
            printf("Stream end token found\n");
            yaml_token_delete(&token);
            break;
        }

        if (token.type != YAML_SCALAR_TOKEN) {
            yaml_token_delete(&token);
            continue;
        }

        if (!found_log && token.data.scalar.value != NULL &&
            strlen((char*)token.data.scalar.value) > 0 &&
            strcmp((char *)token.data.scalar.value, "log") == 0) {

            printf("\"log\" found.\n");
            found_log = true;
            yaml_token_delete(&token);
            continue;
        }

        if(found_log && !found_level && token.data.scalar.value != NULL &&
            strlen((char*)token.data.scalar.value) > 0 &&
            strcmp((char *)token.data.scalar.value, "level") == 0) {

            found_level = true;
            yaml_token_delete(&token);
            continue;
        }

        if(found_log && found_level && strlen(level) == 0 && token.data.scalar.value != NULL &&
            strlen((char*)token.data.scalar.value) > 0) {

            printf("level found: %s\n", (char *)token.data.scalar.value);
            strcpy(level, (char *)token.data.scalar.value);
            *l_level = string_to_log_level((char *)token.data.scalar.value);
            yaml_token_delete(&token);
            continue;
        }

        if(found_log && !found_pattern && token.data.scalar.value != NULL &&
            strlen((char*)token.data.scalar.value) > 0 &&
            strcmp((char *)token.data.scalar.value, "pattern") == 0) {

            found_pattern = true;
            yaml_token_delete(&token);
            continue;
        }

        if(found_log && found_pattern && strlen(pattern) == 0 && token.data.scalar.value != NULL &&
            strlen((char*)token.data.scalar.value) > 0) {

            printf("pattern found: %s\n", (char *)token.data.scalar.value);
            strcpy(pattern, (char *)token.data.scalar.value);
            yaml_token_delete(&token);
            continue;
        }

        if(found_log && !found_filename && token.data.scalar.value != NULL &&
            strlen((char*)token.data.scalar.value) > 0 &&
            strcmp((char *)token.data.scalar.value, "filename") == 0) {

            found_filename = true;
            yaml_token_delete(&token);
            continue;
        }

        if(found_log && found_filename && strlen(filename) == 0 && token.data.scalar.value != NULL &&
            strlen((char*)token.data.scalar.value) > 0) {

            printf("filename found: %s\n", (char *)token.data.scalar.value);
            strcpy(filename, (char *)token.data.scalar.value);
            yaml_token_delete(&token);
            continue;
        }

        if(found_log && !found_enableFile && token.data.scalar.value != NULL &&
            strlen((char*)token.data.scalar.value) > 0 &&
            strcmp((char *)token.data.scalar.value, "enableFile") == 0) {

            found_enableFile = true;
            yaml_token_delete(&token);
            continue;
        }

        if(found_log && found_enableFile && token.data.scalar.value != NULL &&
            strlen((char*)token.data.scalar.value) > 0) {

            printf("enableFile found: %s\n", (char *)token.data.scalar.value);
            *enableFile = strcmp((char *)token.data.scalar.value, "true") == 0;
            yaml_token_delete(&token);
            continue;
        }

        if(found_log && !found_enableConsole && token.data.scalar.value != NULL &&
            strlen((char*)token.data.scalar.value) > 0 &&
            strcmp((char *)token.data.scalar.value, "enableConsole") == 0) {

            found_enableConsole = true;
            yaml_token_delete(&token);
            continue;
        }

        if(found_log && found_enableConsole && token.data.scalar.value != NULL &&
            strlen((char*)token.data.scalar.value) > 0) {

            printf("enableConsole found: %s\n", (char *)token.data.scalar.value);
            *enableConsole = strcmp((char *)token.data.scalar.value, "true") == 0;
            yaml_token_delete(&token);
            continue;
        }
    }

    printf("log read.\n");
    printf("Level: %s\n", level);
    printf("Pattern: %s\n", pattern);
    printf("Filename: %s\n", filename);
    printf("EnableFile: %s\n", *enableFile?"true":"false");
    printf("EnableConsole: %s\n", *enableConsole?"true":"false");

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    fclose(file);

    set_default_log(level, pattern, filename, enableFile, enableConsole);

    *l_level = string_to_log_level(level);

    return 0;
}

int log_setup(const char *config_file, log4c_category_t* category) {

    char pattern[100], filename[100];
    bool enableFile, enableConsole;
    log_levels level;

    if (read_log_config(config_file, &level, pattern, filename, &enableFile, &enableConsole) != 0) {
        fprintf(stderr, "Failed to read log config\n");
        return 1;
    }

    //log4c_layout_t *custom_layout = log4c_layout_get("pattern");

    printf("Pattern: %s\n", pattern);

    //log4c_layout_set_udata(custom_layout, (void *)pattern);

    log4c_category_log(category, LOG4C_PRIORITY_DEBUG, "Config file parsed\n");

    //log4c_appender_t *appender_stdout = log4c_appender_get("stdout");
    //log4c_appender_set_layout(appender_stdout, custom_layout);

    //log4c_appender_t *appender_stderr = log4c_appender_get("stderr");
    //log4c_appender_set_layout(appender_stderr, custom_layout);

    enableFile = false;
    /*log4c_appender_t *appender_file = log4c_appender_get("file");
    log4c_appender_set_layout(appender_file, custom_layout);
    log4c_appender_set_udata(appender_file, (void *)filename);*/

    //log4c_category_set_priority(category, get_log4c_level(level));
    if (enableConsole) {
        //log4c_category_set_appender(category, appender_stdout);
        //log4c_category_set_appender(category, appender_stderr);
        printf("Console enabled\n");
    }

    if (enableFile) {
        //log4c_category_set_appender(category, appender_file);
        printf("File enabled\n");
    }

    printf("log4c setup completed.\n");
    log4c_category_log(category, LOG4C_PRIORITY_DEBUG, "TEST Logger initialized\n");
    printf("Dovresti vedere TEST Logger initialized\n");

    return 0;
}