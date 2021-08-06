# Windows NT 4.0 RAMDISK SCSI miniport driver
This is an archive of the driver originally written by Gary Nebbett around 2002 that allows Windows NT 4.0 to use a ramdisk as a primary boot device.
## Usage
rename the file to ntbootdd.sys and place it somewhere where NTLDR can find it. also install it into your system32\drivers directory so Windows NT can find it while it is starting up.
## how to build
Install Windows NT 4.0, the Windows NT 4 DDK, NT 4 SDK and Visual C++ 4.1. use the tools provided in the ddk to compile from there
## original readme by garry

```
Booting NT from Floppies
========================

When Microsoft knowledge base speaks of booting NT from floppies, it is
normally referring only to the first phase of booting - the loading of ntldr
(and perhaps ntbootdd.sys) and reading of boot.ini. Once these components
have been loaded from floppy, the rest of the files needed to boot and start
NT are loaded from a hard disk. This article describes how to boot NT from
floppies alone - it is not even necessary for a hard disk to be present in
the PC. The mini NT system which is started is able to log users on/off and
uses cmd.exe as the user's shell.

The basic process which will be described is:

- Boot MSDOS and create a RAM disk
- Unzip a stripped down NT system on the RAM disk
- Reboot from the RAM disk

Using this technique it is possible to boot Windows NT 4.0 SP5 from four
1.44MB floppy disks in about 5 minutes. The four floppy disks do not
include utilities such as regedt32, chkdsk or winfile which might be needed
to repair a system, but these can be loaded from other floppies after the
boot is complete; alternatively if the NT installation on the hard disk of
the PC is not too badly damaged, it might be possible to access these
utilities from there.

The position of the RAM disk in memory is stored in three places:

- MSDOS config.sys (creates the RAM disk)
- boot.ini (/maxmem flag stops NT using RAM disk memory)
- ntbootdd.sys (the RAM disk device driver)

It is relatively easy to change its position (three edits and one build would
be needed), but reserving the memory above 16MB for the RAM disk seems like a
reasonable choice.

Since the RAM disk must be at least 12MB in size, the hardware requirements
for the PC to be bootable from floppies are a floppy drive and at least 28MB
of RAM (24MB might just be enough, if only 10-12MB of RAM were to be reserved
for NT). The smallest configuration that I have tested is 16MB for NT and a
16MB RAM disk.


Creating the RAM disk
---------------------

The RAM disk can be created with the MSDOS ramdrive.sys. The following two
lines from config.sys reserve 16MB RAM for NT and create a 16MB RAM disk:

DEVICE=HIMEM.SYS /int15=15360
DEVICE=RAMDRIVE.SYS 16384 /e

Assuming that config.sys does not place any additional demands on extended
memory, the RAM disk created starts exactly at 0x1000000 (the 16MB
boundary). The MSDOS startup menu can be used to select larger RAM disk sizes
(up to the 32MB limit of ramdrive.sys) if the PC has enough memory.


Content of the RAM disk
-----------------------

The files which need to be loaded into the RAM disks are listed below. Some
of the smaller files may be unnecessary (I did not test whether NT would run
without null.sys, for example, although I expect that it would). The list
includes atdisk.sys and atapi.sys so that any IDE/EIDE disks (or CDROM
drives) can be accessed after the boot. If necessary, some SCSI miniports for
common SCSI disk devices could be added to the list.


Files used in approximate load order Reason for loading
------------------------------------ ------------------

\winnt\system32\ntoskrnl.exe Boot
\winnt\system32\hal.dll
\winnt\system32\config\system
\winnt\system32\l_intl.nls
\winnt\system32\c_1252.nls
\winnt\system32\c_850.nls
\winnt\system32\drivers\ntbootdd.sys Start = 0 (Boot) drivers
\winnt\system32\drivers\scsiport.sys
\winnt\system32\drivers\disk.sys
\winnt\system32\drivers\class2.sys
\winnt\system32\drivers\fastfat.sys

\winnt\system32\ntdll.dll

\winnt\system32\drivers\atapi.sys Start = 1 (System) drivers
\winnt\system32\drivers\atdisk.sys
\winnt\system32\drivers\floppy.sys
\winnt\system32\drivers\cdfs.sys
\winnt\system32\drivers\cdrom.sys
\winnt\system32\drivers\fs_rec.sys
\winnt\system32\drivers\i8042prt.sys
\winnt\system32\drivers\kbdclass.sys
\winnt\system32\drivers\ksecdd.sys
\winnt\system32\drivers\mouclass.sys
\winnt\system32\drivers\msfs.sys
\winnt\system32\drivers\npfs.sys
\winnt\system32\drivers\ntfs.sys
\winnt\system32\drivers\null.sys
\winnt\system32\drivers\vga.sys
\winnt\system32\drivers\videoprt.sys

\winnt\system32\smss.exe Started by ntoskrnl

\winnt\system32\advapi32.dll KnownDLLs
\winnt\system32\gdi32.dll
\winnt\system32\kernel32.dll
\winnt\system32\msvcrt.dll
\winnt\system32\rpcrt4.dll
\winnt\system32\user32.dll

\winnt\system32\config\sam
\winnt\system32\config\security
\winnt\system32\config\software

\winnt\system32\win32k.sys Kmode SubSystems

\winnt\system32\csrss.exe Required SubSystems
\winnt\system32\csrsrv.dll
\winnt\system32\basesrv.dll
\winnt\system32\winsrv.dll
\winnt\system32\unicode.nls
\winnt\system32\locale.nls
\winnt\system32\ctype.nls
\winnt\system32\sortkey.nls
\winnt\system32\sorttbls.nls

\winnt\system32\vga.dll
\winnt\system32\kbdus.dll

\winnt\system32\winlogon.exe Started by CSRSS
\winnt\system32\userenv.dll
\winnt\system32\shell32.dll
\winnt\system32\comctl32.dll
\winnt\system32\netapi32.dll
\winnt\system32\netrap.dll
\winnt\system32\samlib.dll
\winnt\system32\winmm.dll
\winnt\system32\msgina.dll
\winnt\system32\rpcltc1.dll
\winnt\system32\rpclts1.dll

\winnt\system32\services.exe Started by Winlogon
\winnt\system32\umpnpmgr.dll

\winnt\system32\lsass.exe Started by Winlogon
\winnt\system32\lsasrv.dll
\winnt\system32\samsrv.dll
\winnt\system32\msprivs.dll
\winnt\system32\netlogon.dll
\winnt\system32\msv1_0.dll
\winnt\system32\security.dll
\winnt\system32\wsock32.dll
\winnt\system32\ws2_32.dll
\winnt\system32\ws2help.dll

\winnt\system32\cmd.exe Started by Winlogon
\winnt\system32\mpr.dll (if Userinit = "cmd")


The list includes four registry hives, some of which are often quite
large. Any source of these hives is acceptable and a good choice is the hives
created during a new installation of NT (the hives are still quite small at
this point). The drawbacks of using large hives are that more floppies may
be needed to contain them and the corresponding additional delay when
copying them to the RAM disk.

Assuming that the files listed above are zipped into a file called nt.zip,
then the contents of the four floppies are:

Disk1 Disk2 Disk3 Disk4

(MSDOS boot sector) (NT boot sector)
io.sys nt.zip nt.zip boot.ini
msdos.sys ntbootdd.sys
command.com ntdetect.com
config.sys ntldr
autoexec.bat reboot.com
himem.sys nt.zip
ramdrive.sys
unzip.exe
nt.zip

and boot.ini contains:

[boot loader]
default=scsi(0)disk(0)rdisk(0)partition(1)\winnt
[operating systems]
scsi(0)disk(0)rdisk(0)partition(1)\winnt="Mini NT 4.0" /maxmem=16 /basevideo


Registry Settings
-----------------

There are a number of values in the system registry hive which need
modification:

HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\Userinit should be
set to "cmd" so that a command window will be started as the user's shell.

HKLM\SYSTEM\CurrentControlSet\Control\Nls\CodePage\ACP and
HKLM\SYSTEM\CurrentControlSet\Control\Nls\CodePage\OEMCP should either
be set to 1252 and 850 respectively, or the files c_1252.nls and c_850.nls in
the zip file should be replaced by code page files which match the existing
registry values.

HKLM\SYSTEM\CurrentControlSet\Control\CrashControl\CrashDumpEnabled and
HKLM\SYSTEM\CurrentControlSet\Control\CrashControl\LogEvent should be
set to zero. This prevents the system from preparing for a crash dump. As a
side effect of making the RAM disk appear to the system as a hard disk,
"partition0" of the RAM disk appears to contain a FAT file system; ntoskrnl
expects this partition to contain a RAW file system and this mismatch leads
to a crash when preparing for a crash dump.

HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management\PagingFiles
should be set to "C:\winnt\system32\temppf.sys 2 8". This will prevent
winlogon from filling the RAM disk with a temporary paging file.

HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management\SystemPages
should be set to 0x3000. This will ensure that there are enough system PTEs
to map a 32MB RAM disk.

HKLM\SYSTEM\CurrentControlSet\Services should be checked for "Start=0"
services. Only ntbootdd and disk should start at boot time; other services
which have "Start=0" should be modified to "Start=1". The entry for ntbootdd
can be created manually if necessary, taking another SCSI miniport as
template (e.g. Aha154x).


Booting from the RAM Disk
-------------------------

After the RAM disk has been loaded, the next step is to reboot the PC from a
"normal" NT boot floppy (Disk4 in the table above) whose boot.ini selects the
RAM disk as the boot disk. The only problem is that the normal Ctrl-Alt-Del
reboot zeros memory, erasing the contents of the RAM disk. Fortunately, there
is a method of rebooting without zeroing memory - by executing the Intel x86
instruction "int 19h" to invoke the bootstrap loader service; quoting from
Ralf Brown's "Interrupt List":

"This interrupt reboots the system without clearing memory or restoring
interrupt vectors."

A two byte .com file containing the values 0xCD, 0x19 does the trick (these
values being the hexadecimal code for "int 19h"); a suitable name for this
file is reboot.com.


Ntbootdd
--------

If boot.ini selects a boot device whose ARC name starts with "scsi", then
there must be a file in the same directory as boot.ini with the name
"ntbootdd.sys". Normally ntbootdd.sys is a renamed copy of the appropriate
SCSI miniport driver for the hard disks of the system, but when booting from
the RAM disk, ntbootdd.sys is built from ntbootdd.cpp (Listing 1).

ntbootdd.sys appears twice on the boot floppies: once on the "normal" NT boot
floppy (where it is used by ntldr to load the initial system components) and
once in the zip file (from whence it is later loaded and used by ntoskrnl to
access the RAM disk). The same file is used in both places, but this is not
compulsory; there are some differences between the two environments into
which ntbootdd.sys is loaded and if necessary this could be compensated for
by alternative versions of the code.

Important environmental differences are that the ntldr implementation of
ScsiPortFreeDeviceBase() does nothing and the implementation of
ScsiPortGetDeviceBase() can map a maximum of 4MB and has problems mapping
non-paged aligned regions that straddle pages. The ntldr implementation of
ScsiPortGetDeviceBase() can be called from the HwScsiStartIo routine, but the
ntoskrnl implementation cannot (because of IRQL considerations).
ntbootdd.sys detects which environment it is in (by examining the first
sector of the RAM disk) and adapts its behaviour accordingly.

The 4MB mapping limit places an upper bound on how much data ntbootdd.sys can
load; excluding the system hive, about 1.3MB needs to be mapped so the system
hive can be as large as 2.5MB (the system hive can be reduced to about 64kB
by deleting unnecessary keys and values).

Ntbootdd was debugged by using the checked version of ntldr; when using the
checked version, the ScsiPort routine ScsiDebugPrint() is available and the
level of output is controlled by the [debug] section in boot.ini. Setting
scsidebug=3 produces a great deal of output; scsidebug=2 is much more
manageable. Since the debug output is only visible on the screen and scrolls
very quickly, it is often helpful to follow a ScsiDebugPrint() with code
like:

for (int i = 0; i < 2000; i++) ScsiPortStallExecution(1000);

Since the RAM disk does not contain a partition table, the initial version of
ntbootdd emulated a floppy device. This mostly worked, but attempts to create
paging files on the RAM disk failed with the error STATUS_FLOPPY_VOLUME ("The
paging file cannot be created on a floppy diskette"). The current version
emulates a hard disk and modifies the first sector of the RAM disk to include
a partition table. Another change is needed to the first sector of the RAM
disk because the MDDOS ramdrive.sys does not format the BIOS parameter blocks
in quite the same way as is expected of a FAT file system; it is necessary to
explicitly set the "Large Sectors" field (offset 0x20) to zero, otherwise the
RAM disk is not recognized as containing a FAT file system.


Drive Letters
-------------

The RAM disk is assigned the drive letter C:. Any other disks for which
drivers have been loaded are assigned drive letters, but these letters will
probably differ from the normal assignment for any particular PC. This is
because C: is already taken, there are no sticky drive letter assignments and
drivers for some disks are not loaded or are loaded in a different order.


Keyboard Layout
---------------

Information about the keyboard layout is stored in the registry on a per-user
basis. When no user is logged in or the logged in user's hive is unavailable,
the keyboard layout information is retrieved from HKEY_USERS\.Default (which
is stored in the hive named "default"). Since neither the default hive nor
any user hives are included in the zip file, the system defaults the keyboard
layout. Naturally, the default keyboard layout is a US keyboard layout. If
this is a problem, a suitable default hive could be included in the zip file.
```
