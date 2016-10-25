#pragma once

#include <string.h> /*for size_t*/

void createPath(const char *Path);
void loadFile(void** out, size_t* outsize, const char* filename);
void saveFile(const void* buffer, size_t buffersize, const char* filename);
