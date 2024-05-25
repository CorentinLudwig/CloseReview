#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utils/config.h>
#include <utils/logger.h>

#define FILE_CONFIG "config.c"

bool getConfigFilePath(char config_file[DIRECTORY_MAX_SIZE]) {
    char FUN_NAME[32] = "getConfigFilePath";
    char current_dir[DIRECTORY_MAX_SIZE];
    memset(config_file, 0, DIRECTORY_MAX_SIZE);
    if (getcwd(current_dir, DIRECTORY_MAX_SIZE) == NULL) {
        warnl(FILE_CONFIG, FUN_NAME, "Couldn't get current working directory");
        return false;
    }
    char *needle = strstr(current_dir, "close-review");
    if (needle == NULL) {
        warnl(FILE_CONFIG, FUN_NAME, "Directory 'close-review' not found in the current path");
        return false;
    }
    needle += strlen("close-review");
    memset(needle, '\0', DIRECTORY_MAX_SIZE - (needle - current_dir));
    snprintf(needle, sizeof("/config.toml"), "/config.toml");
    strncpy(config_file, current_dir, strnlen(current_dir, DIRECTORY_MAX_SIZE));
    config_file[DIRECTORY_MAX_SIZE - 1] = '\0';
    return true;
}

t_config *initConfig(FILE *config) {
    char FUN_NAME[32] = "initConfig";
    char header[BUFFER_SIZE];
    char token[BUFFER_SIZE];
    t_config *configuration = malloc(sizeof(t_config));

    while (fgets(header, sizeof(header), config)) {
        if (sscanf(header, "[ %[^]] ]", token) == 1) {
            if (strncmp(token, "user", BUFFER_SIZE) == 0) {
                configuration->user.is_defined =
                    fscanf(config, "ip = \"%15[^\"]\"\n", configuration->user.ip) == 1 &&
                    fscanf(config, "local_port = %d\n", &configuration->user.local_port) == 1 &&
                    fscanf(config, "public_port = %d\n", &configuration->user.public_port) == 1;
                if (!configuration->user.is_defined) {
                    warnl(FILE_CONFIG, FUN_NAME, "Error reading user section");
                    configuration->user.ip[0] = '\0';
                    configuration->user.local_port = -1;
                    configuration->user.public_port = -1;
                }
            } else if (strncmp(token, "server", BUFFER_SIZE) == 0) {
                configuration->server.is_defined =
                    fscanf(config, "ip = \"%15[^\"]\"\n", configuration->server.ip) == 1 &&
                    fscanf(config, "port = %d\n", &configuration->server.port) == 1;
                if (!configuration->server.is_defined) {
                    warnl(FILE_CONFIG, FUN_NAME, "Error reading user section");
                    configuration->server.ip[0] = '\0';
                    configuration->server.port = -1;
                }
            } else if (strncmp(token, "history", BUFFER_SIZE) == 0) {
                char consent[BUFFER_SIZE];
                configuration->history.is_defined =
                    fscanf(config, "user_consent = %s\n", consent) == 1 &&
                    fscanf(config, "path = \"%[^\"]\"\n", configuration->history.path) == 1;
                if (!configuration->history.is_defined) {
                    warnl(FILE_CONFIG, FUN_NAME, "Error reading user section");
                    configuration->history.user_consent = false;
                    configuration->history.path[0] = '\0';
                }
                configuration->history.user_consent = (strcmp(consent, "true") == 0);
            } else if (strncmp(token, "ssl", BUFFER_SIZE) == 0) {
                configuration->config_ssl.is_defined =
                    fscanf(config, "certificate = \"%[^\"]\"\n", configuration->config_ssl.certificate) == 1 &&
                    fscanf(config, "key = \"%[^\"]\"\n", configuration->config_ssl.key) == 1;
                if (configuration->config_ssl.is_defined) {
                    warnl(FILE_CONFIG, FUN_NAME, "Error reading user section");
                    configuration->config_ssl.certificate[0] = '\0';
                    configuration->config_ssl.key[0] = '\0';
                }
            }
        }
    }

    if (ferror(config)) {
        fclose(config);
        warnl(FILE_CONFIG, FUN_NAME, "Error reading config file");
        return NULL;
    }

    return configuration;
}
