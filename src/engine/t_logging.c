#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef enum t_log_level { INFO, DEBUG, WARNING, ERROR } t_log_level;

static void s_log(t_log_level log_level, const char* text, va_list args) {

    char buffer[256] = { 0 };
    switch (log_level) { 
        case INFO:  strcpy(buffer, "Info: "); break;
        case DEBUG:  strcpy(buffer, "DEBUG: "); break;
        case WARNING:  strcpy(buffer, "Warning: "); break;
        case ERROR:  strcpy(buffer, "Error: "); break;
    }

    unsigned int text_size = (unsigned int)strlen(text);
    memcpy(buffer + strlen(buffer), text, text_size);
    strcat(buffer, "\n");
    vprintf(buffer, args);
}

void t_log_info(const char* text, ...) {
    va_list args;
    va_start(args, text);
    s_log(INFO, text, args);
    va_end(args);
}

void t_log_debug(const char* text, ...) { 
    va_list args;
    va_start(args, text);
    s_log(DEBUG, text, args);
    va_end(args);
}

void t_log_warning(const char* text, ...) {
    va_list args;
    va_start(args, text);
    s_log(WARNING, text, args);
    va_end(args);
}

void t_log_error(const char* text, ...) {
    va_list args;
    va_start(args, text);
    s_log(ERROR, text, args);
    va_end(args);
}