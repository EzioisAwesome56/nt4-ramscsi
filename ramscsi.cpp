extern "C" {
#include "miniport.h"
#include "scsi.h"
}
#include <memory.h>

#pragma pack(push, 1)

typedef struct PTABLE {
UCHAR Flag;
UCHAR Chs1[3];
UCHAR Type;
UCHAR Chs2[3];
ULONG Start;
ULONG Size;
} *PPTABLE;

typedef struct FAT {
UCHAR Jump[3];
UCHAR System[8];
USHORT BytesPerSector;
UCHAR SectorsPerCluster;
USHORT ReservedSectors;
UCHAR FatCopies;
USHORT RootEntries;
USHORT SmallSectors;
UCHAR MediaType;
USHORT SectorsPerFat;
USHORT SectorsPerTrack;
USHORT Heads;
ULONG HiddenSectors;
ULONG LargeSectors;
UCHAR Disk;
UCHAR Flags;
UCHAR Signature;
ULONG SerialNumber;
UCHAR Label[11];
UCHAR Format[8];
UCHAR Code[0x17A];
ULONG DiskSignature;
UCHAR Reserved;
UCHAR BootPartition;
PTABLE Table[4];
USHORT BootSignature;
} *PFAT;

#pragma pack(pop)


typedef PVOID *PPVOID;

const ULONG PageSize = 0x1000;
const ULONG DiskBase = 0x1000000;

ULONG BytesPerBlock, Blocks;
PCHAR MapBase;
BOOLEAN Boot;


ULONG ChunkSize(ULONG Offset, ULONG Length, ULONG Position)
{
ULONG X = PageSize - ((Offset + Position) & (PageSize - 1));
ULONG Y = Length - Position;

return X < Y ? X : Y;
}


PVOID MapPage(PVOID DeviceExtension, ULONG Address)
{
ULONG X = (Address - DiskBase) / PageSize;

if (PPVOID(DeviceExtension)[X] == 0)

*(PPVOID(DeviceExtension) + X) =

ScsiPortGetDeviceBase(DeviceExtension, Internal, 0,
ScsiPortConvertUlongToPhysicalAddress(Address & ~(PageSize - 1)),
PageSize, FALSE);

return PCHAR(PPVOID(DeviceExtension)[X]) + (Address & (PageSize - 1));
}


PVOID MapAddress(PVOID DeviceExtension, ULONG Address, ULONG Size)
{
return ScsiPortGetDeviceBase(DeviceExtension, Internal, 0,
ScsiPortConvertUlongToPhysicalAddress(Address), Size, FALSE);
}


BOOLEAN StartIo(PVOID DeviceExtension, PSCSI_REQUEST_BLOCK Srb)
{
Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
Srb->ScsiStatus = SCSISTAT_GOOD;

if (Srb->Function == SRB_FUNCTION_EXECUTE_SCSI && Srb->Lun == 0) {

switch (Srb->Cdb[0]) {

case SCSIOP_INQUIRY:
{
memset(Srb->DataBuffer, 0, Srb->DataTransferLength);
PINQUIRYDATA Inq = PINQUIRYDATA(Srb->DataBuffer);
Inq->DeviceType = DIRECT_ACCESS_DEVICE;
Inq->DeviceTypeQualifier = DEVICE_CONNECTED;
memcpy(Inq->VendorId, "NEBBETT ", sizeof Inq->VendorId);
Srb->SrbStatus = SRB_STATUS_SUCCESS;
}
break;

case SCSIOP_MODE_SENSE:
{
memset(Srb->DataBuffer, 0, Srb->DataTransferLength);
PMODE_PARM_READ_WRITE_DATA Mode = PMODE_PARM_READ_WRITE_DATA(Srb->DataBuffer);
Mode->ParameterListHeader.ModeDataLength = sizeof *Mode - 1;
Mode->ParameterListHeader.BlockDescriptorLength = sizeof Mode->ParameterListBlock;
Srb->SrbStatus = SRB_STATUS_SUCCESS;
}
break;

case SCSIOP_TEST_UNIT_READY:
case SCSIOP_MEDIUM_REMOVAL:
{
Srb->SrbStatus = SRB_STATUS_SUCCESS;
}
break;

case SCSIOP_READ_CAPACITY:
{
ULONG X = BytesPerBlock;
REVERSE_BYTES(&PREAD_CAPACITY_DATA(Srb->DataBuffer)->BytesPerBlock, &X);
X = Blocks - 1;
REVERSE_BYTES(&PREAD_CAPACITY_DATA(Srb->DataBuffer)->LogicalBlockAddress, &X);
Srb->SrbStatus = SRB_STATUS_SUCCESS;
}
break;

case SCSIOP_READ:
{
ULONG X;
REVERSE_BYTES(&X, &PCDB(Srb->Cdb)->CDB10.LogicalBlockByte0);

if (Boot) {
X = X * BytesPerBlock + DiskBase;
for (ULONG Z, Y = 0; Y < Srb->DataTransferLength; Y += Z) {
Z = ChunkSize(X, Srb->DataTransferLength, Y);
memcpy(PCHAR(Srb->DataBuffer) + Y, MapPage(DeviceExtension, X + Y), Z);
}
}
else
memcpy(Srb->DataBuffer, MapBase + X * BytesPerBlock, Srb->DataTransferLength);
Srb->SrbStatus = SRB_STATUS_SUCCESS;
}
break;

case SCSIOP_WRITE:
{
ULONG X;
REVERSE_BYTES(&X, &PCDB(Srb->Cdb)->CDB10.LogicalBlockByte0);

if (Boot) {
X = X * BytesPerBlock + DiskBase;
for (ULONG Z, Y = 0; Y < Srb->DataTransferLength; Y += Z) {
Z = ChunkSize(X, Srb->DataTransferLength, Y);
memcpy(MapPage(DeviceExtension, X + Y), PCHAR(Srb->DataBuffer) + Y, Z);
}
}
else
memcpy(MapBase + X * BytesPerBlock, Srb->DataBuffer, Srb->DataTransferLength);
Srb->SrbStatus = SRB_STATUS_SUCCESS;
}
break;

default:
{
ScsiDebugPrint(1, "StartIo Function Execute %x, Flags %lx, Len %lx\n",
int(Srb->Cdb[0]), Srb->SrbFlags, Srb->DataTransferLength);
// for (int i = 0; i < 2000; i++) ScsiPortStallExecution(1000);
}
break;
}
}

ScsiPortNotification(RequestComplete, DeviceExtension, Srb);
ScsiPortNotification(NextRequest, DeviceExtension);

return TRUE;
}


ULONG FindAdapter(PVOID DeviceExtension, PVOID, PVOID, PCHAR,
PPORT_CONFIGURATION_INFORMATION ConfigInfo, PBOOLEAN Again)
{
PFAT Fat = PFAT(MapAddress(DeviceExtension, DiskBase, sizeof (FAT)));

Boot = Fat->LargeSectors != 0;

BytesPerBlock = Fat->BytesPerSector;
Blocks = Fat->SmallSectors;

Fat->LargeSectors = 0;
Fat->BootSignature = 0xAA55;
Fat->Table[0].Type = 4;
Fat->Table[0].Start = 0;
Fat->Table[0].Size = Blocks;
Fat->Table[1].Type = Fat->Table[2].Type = Fat->Table[3].Type = 0;

ScsiPortFreeDeviceBase(DeviceExtension, Fat);

if (!Boot) MapBase = PCHAR(MapAddress(DeviceExtension, DiskBase, Blocks * BytesPerBlock));

ConfigInfo->NumberOfBuses = 1;
ConfigInfo->MaximumNumberOfTargets = 1;

*Again = FALSE;

return SP_RETURN_FOUND;
}


BOOLEAN ResetBus(PVOID DeviceExtension, ULONG PathId)
{
ScsiPortCompleteRequest(DeviceExtension, UCHAR(PathId), SP_UNTAGGED, SP_UNTAGGED,
SRB_STATUS_BUS_RESET);

return TRUE;
}


BOOLEAN Initialize(PVOID)
{
return TRUE;
}


extern "C"
ULONG DriverEntry(PVOID DriverObject, PVOID RegistryPath)
{
HW_INITIALIZATION_DATA Hid = {sizeof Hid};

Hid.AdapterInterfaceType = Isa;
Hid.DeviceExtensionSize = 0x2000 * sizeof (PVOID);
Hid.MapBuffers = TRUE;

Hid.HwFindAdapter = FindAdapter;
Hid.HwInitialize = Initialize;
Hid.HwResetBus = ResetBus;
Hid.HwStartIo = StartIo;

return ScsiPortInitialize(DriverObject, RegistryPath, &Hid, 0);
} 
