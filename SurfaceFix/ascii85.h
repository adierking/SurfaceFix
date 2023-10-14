#pragma once
#include "stdafx.h"

extern std::string Encode85(const BYTE* buffer, int length);
extern int Decode85(const char* buffer, int length, std::string& dest);