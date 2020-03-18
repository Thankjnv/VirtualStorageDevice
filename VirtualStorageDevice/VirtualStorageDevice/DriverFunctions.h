#pragma once
#include "Common.h"

NTSTATUS unimplementedMajorFunction(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchCreate(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchClose(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchCleanup(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchIoctl(PDEVICE_OBJECT deviceObject, PIRP irp);
