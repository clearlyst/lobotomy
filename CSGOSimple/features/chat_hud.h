#pragma once

#include <cstdarg>
#include <stdio.h>
#include "../valve_sdk/misc/vfunc.hpp"
#define MAX_BUFFER_SIZE 1024

#define CHAT_COLOR_DEFAULT "\x01"
#define CHAT_COLOR_RED "\x02"
#define CHAT_COLOR_LIGHTPURPLE "\x03"
#define CHAT_COLOR_GREEN "\x04"
#define CHAT_COLOR_LIME "\x05"
#define CHAT_COLOR_LIGHTGREEN "\x06"
#define CHAT_COLOR_LIGHTRED "\x07"
#define CHAT_COLOR_GRAY "\x08"
#define CHAT_COLOR_LIGHTOLIVE "\x09"
#define CHAT_COLOR_LIGHTSTEELBLUE "\x0A"
#define CHAT_COLOR_LIGHTBLUE "\x0B"
#define CHAT_COLOR_BLUE "\x0C"
#define CHAT_COLOR_PURPLE "\x0D"
#define CHAT_COLOR_PINK "\x0E"
#define CHAT_COLOR_LIGHTRED2 "\x0F"
#define CHAT_COLOR_OLIVE "\x10"

class c_hudchat {
public:

	void chatprintf(int iPlayerIndex, int iFilter, const char* format, ...)
	{
		static char buf[MAX_BUFFER_SIZE] = "";
		va_list va;
		va_start(va, format);
		vsnprintf_s(buf, MAX_BUFFER_SIZE, format, va);
		va_end(va);
		CallVFunction<void(__cdecl*)(void*, int, int, const char*, ...)>(this, 27)(this, iPlayerIndex, iFilter, buf);
	}
}; 

