#pragma once
// Before including this file you must first include "Windows.h" for user mode or "ntddk.h" for kernel mode

#define MY_VENDOR_CODE 0x8000
#define IOCTL_TEST_CODE CTL_CODE(MY_VENDOR_CODE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

