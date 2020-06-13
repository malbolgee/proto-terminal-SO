#pragma once

#define MAX_ENTRY_LENGTH 8192

#define REGEX_PATTERN_RUNPROGRAM "^\\.\\/[a-zA-Z0-9_]+$"
#define REGEX_PATTERN_ARGUMENT "^-[a-zA-Z]+$"
#define REGEX_PATTERN_FILENAME "^[a-zA-Z0-9_]+$"

int parserSO (char *);
int commandCheckerSO (char *, const char **, unsigned);
int modifierCheckerSO (char *, const char **, unsigned);
int typeChecker (char *, regex_t, regex_t, regex_t);
int patternCheckerSO (const char *, regex_t);
int tokenHandlerSO (char *, char *);