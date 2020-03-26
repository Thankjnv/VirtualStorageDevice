#include "Utils.h"

// {E384886D-C499-4776-A22F-3CD742640409}
const GUID g_deviceClassGuid = { 0xe384886d, 0xc499, 0x4776, { 0xa2, 0x2f, 0x3c, 0xd7, 0x42, 0x64, 0x4, 0x9 } };

volatile ULONG g_currentVirtualStorageDeviceID = 0;
