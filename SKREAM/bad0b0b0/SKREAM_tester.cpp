#include "stdafx.h"
#include <windows.h>

typedef LONG NTSTATUS;

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

#define STATUS_CONFLICTING_ADDRESSES     ((NTSTATUS)0xC0000018L)
#define STATUS_NOT_COMMITTED             ((NTSTATUS)0xC000002DL)
#define STATUS_INVALID_PAGE_PROTECTION   ((NTSTATUS)0xC0000045L)

NTSTATUS NTAPI ZwUnmapViewOfSection(
    HANDLE ProcessHandle,
    PVOID  BaseAddress
);

NTSTATUS NTAPI NtAllocateVirtualMemory(
    HANDLE    ProcessHandle,
    PVOID     *BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T   RegionSize,
    ULONG     AllocationType,
    ULONG     Protect
);

NTSTATUS
NTAPI
NtProtectVirtualMemory(
    IN HANDLE               ProcessHandle,
    IN OUT PVOID            *BaseAddress,
    IN OUT PSIZE_T          NumberOfBytesToProtect,
    IN ULONG                NewAccessProtection,
    OUT PULONG              OldAccessProtection);

NTSTATUS
NTAPI
NtFreeVirtualMemory(
    IN HANDLE               ProcessHandle,
    IN PVOID                *BaseAddress,
    IN OUT PSIZE_T          RegionSize,
    IN ULONG                FreeType);

void try_allocate_bad0b0b0()
{
    static auto pfnNtAllocateVirtualMemory = 
        reinterpret_cast<decltype(NtAllocateVirtualMemory) *>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtAllocateVirtualMemory"));

    PVOID address = reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(0xbad0b0b0));
    SIZE_T size = 0x1000;

    printf("[*] Trying to allocate 0xbad0b0b0. Type = MEM_RESERVE, Protection = PAGE_READONLY\n");
    NTSTATUS status = pfnNtAllocateVirtualMemory(GetCurrentProcess(), &address, 0, &size, MEM_RESERVE, PAGE_READONLY);
    
    if (NT_SUCCESS(status)) {
        printf("[-] 0xbad0b0b0 page was allocated (FAILURE)\n");
    }
    else if (status == STATUS_CONFLICTING_ADDRESSES) {
        printf("[+] 0xbad0b0b0 page could not be allocated (SUCCESS)\n");
    }
    else {
        printf("[?] Unexpected failure status %08x\n", status);
    }

    address = reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(0xbad0b0b0));
    size = 0x1000;

    printf("[*] Trying to allocate 0xbad0b0b0. Type = MEM_COMMIT, Protection = PAGE_READONLY\n");
    status = pfnNtAllocateVirtualMemory(GetCurrentProcess(), &address, 0, &size, MEM_COMMIT, PAGE_READONLY);

    if (NT_SUCCESS(status)) {
        printf("[-] 0xbad0b0b0 page was allocated (FAILURE)\n");
    }
    else if (status == STATUS_CONFLICTING_ADDRESSES) {
        printf("[+] 0xbad0b0b0 page could not be allocated (SUCCESS)\n");
    }
    else {
        printf("[?] Unexpected failure status %08x\n", status);
    }
}

void try_change_protection_bad0b0b0()
{
    static auto pfnNtProtectVirtualMemory =
        reinterpret_cast<decltype(NtProtectVirtualMemory) *>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtProtectVirtualMemory"));

    PVOID address = reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(0xbad0b0b0));
    SIZE_T size = 0x1000;
    ULONG oldProtect = 0;

    printf("Trying to change protection for 0xbad0b0b0. Protection = PAGE_READONLY\n");
    NTSTATUS status = pfnNtProtectVirtualMemory(GetCurrentProcess(), &address, &size, PAGE_READONLY, &oldProtect);

    if (NT_SUCCESS(status)) {
        printf("[-] 0xbad0b0b0 page protection was modified (FAILURE)\n");
    }
    else if ((status == STATUS_INVALID_PAGE_PROTECTION) || (status == STATUS_NOT_COMMITTED)) {
        printf("[+] 0xbad0b0b0 page protection could not be modified (SUCCESS)\n");
    }
    else {
        printf("[?] Unexpected failure status %08x\n", status);
    }

    address = reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(0xbad0b0b0));
    size = 0x1000;

    printf("Trying to change protection for 0xbad0b0b0. Protection = PAGE_READWRITE\n");
    status = pfnNtProtectVirtualMemory(GetCurrentProcess(), &address, &size, PAGE_READWRITE, &oldProtect);\

    if (NT_SUCCESS(status)) {
        printf("[-] 0xbad0b0b0 page protection was modified (FAILURE)\n");
    }
    else if ((status == STATUS_INVALID_PAGE_PROTECTION) || (status == STATUS_NOT_COMMITTED)) {
        printf("[+] 0xbad0b0b0 page protection could not be modified (SUCCESS)\n");
    }
    else {
        printf("[?] Unexpected failure status %08x\n", status);
    }
}

void try_free_bad0b0b0()
{
    static auto pfnNtFreeVirtualMemory =
        reinterpret_cast<decltype(NtFreeVirtualMemory) *>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtFreeVirtualMemory"));

    PVOID address = reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(0x00000000bad0b0b0 & 0xffffffffffff0000));
    SIZE_T size = 0;

    printf("[*] Trying to free 0xbad0b0b0 page\n");
    NTSTATUS status = pfnNtFreeVirtualMemory(GetCurrentProcess(), &address, &size, MEM_RELEASE);

    if (NT_SUCCESS(status)) {
        printf("[-] 0xbad0b0b0 page was freed (FAILURE)\n");
    }
    else if (status == STATUS_INVALID_PAGE_PROTECTION) {
        printf("[+] 0xbad0b0b0 page could not be freed (SUCCESS)\n");
    }
    else {
        printf("[?] Unexpected failure status %08x\n", status);
    }
}

void try_unmap_bad0b0b0()
{
    static auto pfnUnmapViewOfSection =
        reinterpret_cast<decltype(ZwUnmapViewOfSection) *>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "ZwUnmapViewOfSection"));

    PVOID address = reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(0xbad0b0b0));

    printf("[*] Trying to unmap 0xbad0b0b0 page\n");
    NTSTATUS status = pfnUnmapViewOfSection(GetCurrentProcess(), address);

    if (NT_SUCCESS(status)) {
        printf("[-] 0xbad0b0b0 page was unmapped (FAILURE)\n");
    }
    else {
        printf("[+] 0xbad0b0b0 page could not be unmapped (SUCCESS)\n");
    }
}

int main()
{
    // Try to allocate 0xbad0b0b0 by calling NtAllocateVirtualMemory.
    try_allocate_bad0b0b0();

    // Try to change page protection for 0xbad0b0b0 by calling NtProtectVirtualMemory.
    try_change_protection_bad0b0b0();

    // Try to free 0xbad0b0b0 by calling NtFreeVirtualMemory.
    try_free_bad0b0b0();

    // Try to unmap 0xbad0b0b0 by calling NtUnmapViewOfSection.
    try_unmap_bad0b0b0();

    return 0;
}

