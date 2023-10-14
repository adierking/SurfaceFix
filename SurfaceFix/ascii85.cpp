// ascii85.cpp - Provides functions for encoding/decoding Ascii85-ish strings.

#include "stdafx.h"

DWORD powerOf85[5] = {85 * 85 * 85 * 85, 85 * 85 * 85, 85 * 85, 85, 1};

// Encodes a base-85 group
void EncodeGroup85(DWORD group, int length, char* dest)
{
	char chars[5] = {0, 0, 0, 0, 0};
	char* curChar = chars;
	char asciiChar;

	// Special cases
	switch (group)
	{
	case 0x00000000:
		dest[0] = 'z';
		dest[1] = 0;
		return;
	case 0xFFFFFFFF:
		dest[0] = 'y';
		dest[1] = 0;
		return;
	case 0x000000FF:
		dest[0] = 'x';
		dest[1] = 0;
		return;
	case 0x0000FFFF:
		dest[0] = '{';
		dest[1] = 0;
		return;
	case 0x00FF00FF:
		dest[0] = '|';
		dest[1] = 0;
		return;
	case 0xFF0000FF:
		dest[0] = '}';
		dest[1] = 0;
		return;
	}

	// Convert the group to base 85
	for (int i = 0; i < 5; i++)
	{
		asciiChar = group % 85 + 33;
		if (asciiChar == '\"')
		{
			asciiChar = 'w';
		}
		*curChar++ = asciiChar;
		group /= 85;
	}
	for (int i = length; i >= 0; i--)
	{
		*dest++ = *--curChar;
	}
	*dest++ = 0;
}

// Encodes a buffer and returns the result.
std::string Encode85(const BYTE* buffer, int length)
{
	DWORD group = 0, groupPos = 0;
	int i = 0;
	char groupAscii[6];
	std::string result;

	// Encode every byte
	while (i < length)
	{
		// Keep building the group
		switch (groupPos++)
		{
		case 0:
			group |= buffer[i++] << 24;
			break;
		case 1:
			group |= buffer[i++] << 16;
			break;
		case 2:
			group |= buffer[i++] << 8;
			break;
		case 3:
			group |= buffer[i++];

			// The group is full, so encode it
			EncodeGroup85(group, 4, groupAscii);
			result += groupAscii;
			group = 0;
			groupPos = 0;
			break;
		}
	}

	// Take care of unfinished groups
	if (groupPos > 0)
	{
		EncodeGroup85(group, groupPos, groupAscii);
		result += groupAscii;
	}

	return result;
}

// Decodes an encoded buffer.
int Decode85(const char* buffer, int length, std::string& dest)
{
	DWORD group = 0, groupPos = 0;
	char ch;

	// Clear the destination string
	dest.clear();

	// Process each character
	for (int i = 0; i < length; i++)
	{
		ch = buffer[i];
		switch (ch)
		{
		case 'z':
			dest += (char)0;
			dest += (char)0;
			dest += (char)0;
			dest += (char)0;
			break;
		case 'y':
			dest += (char)0xFF;
			dest += (char)0xFF;
			dest += (char)0xFF;
			dest += (char)0xFF;
			break;
		case 'x':
			dest += (char)0;
			dest += (char)0;
			dest += (char)0;
			dest += (char)0xFF;
			break;
		case '{':
			dest += (char)0;
			dest += (char)0;
			dest += (char)0xFF;
			dest += (char)0xFF;
			break;
		case '|':
			dest += (char)0;
			dest += (char)0xFF;
			dest += (char)0;
			dest += (char)0xFF;
			break;
		case '}':
			dest += (char)0xFF;
			dest += (char)0;
			dest += (char)0;
			dest += (char)0xFF;
			break;
		case '\n':
		case '\r':
		case '\t':
		case ' ':
			break;
		case 'w':
			ch = '\"';
		default:
			// Check that the character is valid
			if (ch < 33 || ch > 'u')
			{
				return 0;
			}

			// Add the character to the group
			group += (ch - 33) * powerOf85[groupPos];
			if (groupPos++ == 4)
			{
				// The group is complete - write the bytes out
				dest += (char)((group >> 24) & 0xFF);
				dest += (char)((group >> 16) & 0xFF);
				dest += (char)((group >> 8) & 0xFF);
				dest += (char)(group & 0xFF);
				groupPos = 0;
				group = 0;
			}
		}
	}

	// Process any unfinished groups
	if (groupPos > 0)
	{
		group += powerOf85[--groupPos];
		switch (groupPos)
		{
		case 1:
			dest += (char)((group >> 24) & 0xFF);
			break;
		case 2:
			dest += (char)((group >> 24) & 0xFF);
			dest += (char)((group >> 16) & 0xFF);
			break;
		case 3:
			dest += (char)((group >> 24) & 0xFF);
			dest += (char)((group >> 16) & 0xFF);
			dest += (char)((group >> 8) & 0xFF);
			break;
		}
	}
	return 1;
}