#pragma once

#include <string.h> /*for size_t*/

namespace file {

void createPath(const char *Path);
void loadFile(uint8_t** out, size_t* outsize, const char* filename);
void saveFile(const void* buffer, size_t buffersize, const char* filename);

}

