#pragma once
#include "common.h"

#define RES_PROCEXP_BINARY 1000

/// <summary>
/// Writes the embedded binary file to the specified path
/// </summary>
/// <param name="path">The absolute path to which the binary file should be written </param>
/// <returns></returns>
BOOL WriteResourceToDisk(LPWSTR path);