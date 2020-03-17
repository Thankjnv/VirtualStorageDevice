#pragma once
#include <ntddk.h>

#define TRACE(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, static_cast<ULONG>(-1), "[" __FUNCTION__ ":%d]  " format, __LINE__, __VA_ARGS__)
