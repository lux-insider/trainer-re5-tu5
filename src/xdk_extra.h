// Funções do sistema que o XDK não declara nos cabeçalhos públicos (kernel e
// XAM internos). As que estão em xapilib.lib e xboxkrnl.lib são ligadas pelo
// nome; as outras são procuradas pelo ordinal no módulo carregado.
#pragma once

#include <xtl.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------- XAM
DWORD WINAPI XamGetCurrentTitleId(VOID);
// Lê o controle direto no XAM. O trainer original usa dwFlags = 1; o
// XInputGetState da xapilib não enxerga o controle numa DLL de sistema.
DWORD WINAPI XamInputGetState(DWORD dwUserIndex, DWORD dwFlags, PXINPUT_STATE pState);

// ---------------------------------------------------------------- kernel
LONG  WINAPI XexGetModuleHandle(PCSTR szModuleName, PHANDLE phModule);
LONG  WINAPI XexGetProcedureAddress(HANDLE hModule, DWORD dwOrdinal, PVOID* ppvProc);
LONG  WINAPI XexLoadImage(PCSTR szImageName, DWORD dwFlags, DWORD dwMinVersion, PHANDLE phModule);
BOOL  WINAPI MmIsAddressValid(PVOID pvAddress);
VOID  WINAPI KeSweepDcacheRange(PVOID pvAddress, SIZE_T cb);
VOID  WINAPI KeSweepIcacheRange(PVOID pvAddress, SIZE_T cb);
LONG  WINAPI ExCreateThread(PHANDLE pHandle, DWORD dwStackSize, LPDWORD pdwThreadId,
                            PVOID pApiThreadStartup, LPTHREAD_START_ROUTINE pStartAddress,
                            PVOID pParameter, DWORD dwCreationFlags);
VOID  WINAPI XapiThreadStartup(VOID (__cdecl* pStartRoutine)(PVOID), PVOID pContext);

#define XEX_MODULE_FLAG_DLL         0x00000008
#define EX_CREATE_FLAG_SYSTEM       0x00000002

#ifdef __cplusplus
}
#endif

// ---------------------------------------------------------------- resolvidas na hora
// Não estão nas bibliotecas públicas: são procuradas pelo ordinal no módulo
// carregado (o mesmo que o trainer original usa), e o programa confere se vieram.
#define ORDINAL_XNOTIFYQUEUEUI  656     // xam.xex

typedef VOID    (*PFN_XNOTIFYQUEUEUI)(DWORD dwType, DWORD dwUserIndex, ULONGLONG qwAreas,
                                      PCWSTR wszText, PVOID pContext);

// Procura uma função de um módulo do sistema pelo ordinal; NULL se o módulo
// não estiver carregado ou não tiver esse ordinal.
inline PVOID ResolveOrdinal(PCSTR modulo, DWORD ordinal)
{
    HANDLE h = NULL;
    PVOID funcao = NULL;
    if (XexGetModuleHandle(modulo, &h) < 0 || h == NULL)
        return NULL;
    if (XexGetProcedureAddress(h, ordinal, &funcao) < 0)
        return NULL;
    return funcao;
}

// Notificação do canto da tela (a mesma chamada do trainer original). A
// função é procurada uma vez, ao carregar (como o original faz); dentro do
// jogo a busca pelo nome do módulo pode falhar.
static PFN_XNOTIFYQUEUEUI g_notifica = NULL;

inline VOID ResolveNotifica()
{
    if (g_notifica == NULL)
        g_notifica = (PFN_XNOTIFYQUEUEUI)ResolveOrdinal("xam.xex", ORDINAL_XNOTIFYQUEUEUI);
}

inline VOID Notifica(PCWSTR texto)
{
    ResolveNotifica();
    if (g_notifica != NULL)
        g_notifica(34, 0xFF, 2, texto, NULL);
}
