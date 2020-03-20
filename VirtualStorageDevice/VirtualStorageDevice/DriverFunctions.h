#pragma once
#include "Utils.h"

NTSTATUS unimplementedMajorFunction(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchCreate(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchClose(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchCleanup(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchIoctl(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchRead(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchWrite(PDEVICE_OBJECT deviceObject, PIRP irp);
