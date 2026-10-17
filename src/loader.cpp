// Loader do Trainer RE5 TU5 (abre pelo Aurora).
//
// Carrega o re5trainer.xex da mesma pasta como DLL de sistema e volta ao
// Aurora. O trainer fica na memória até desligar o console.
#include "xdk_extra.h"

static const PCSTR NOME_TRAINER = "re5trainer.xex";
static const PCSTR CAMINHO_TRAINER = "game:\\re5trainer.xex";
// área do re5trainer.xex (base do xex/trainer.xml + tamanho da imagem, com folga)
static const DWORD BASE_TRAINER = 0x91500000;
static const DWORD TAMANHO_TRAINER = 0x30000;

// Escreve em texto as áreas livres (de pelo menos 192 KB) da região de plugins,
// para saber onde o trainer pode morar.
static VOID AreasLivres(WCHAR* texto, int tamanho, int n)
{
    DWORD inicio = 0, achadas = 0;
    for (DWORD a = 0x91000000; a <= 0x91FF0000 && achadas < 4 && n > 0; a += 0x1000) {
        BOOL livre = a < 0x91FF0000 && !MmIsAddressValid((PVOID)a);
        if (livre && inicio == 0)
            inicio = a;
        if (!livre && inicio != 0) {
            if (a - inicio >= 0x30000) {
                n += swprintf_s(texto + n, tamanho - n, L" %08X-%08X", inicio, a);
                achadas++;
            }
            inicio = 0;
        }
    }
}

// A área onde o trainer vai morar está livre?
static BOOL AreaLivre()
{
    for (DWORD a = BASE_TRAINER; a < BASE_TRAINER + TAMANHO_TRAINER; a += 0x1000)
        if (MmIsAddressValid((PVOID)a))
            return FALSE;
    return TRUE;
}

// O console só deixa carregar uma DLL de sistema a partir de uma thread de
// sistema (da thread do aplicativo dá C0000022, acesso negado). Como o loader
// original, o carregamento roda numa thread criada com ExCreateThread.
static volatile LONG g_status = 0;
static volatile LONG g_terminou = 0;

static DWORD WINAPI Carrega(PVOID)
{
    g_status = XexLoadImage(CAMINHO_TRAINER, XEX_MODULE_FLAG_DLL, 0, NULL);
    g_terminou = 1;
    return 0;
}

// Carrega o trainer numa thread de sistema e espera ela terminar.
static LONG CarregaTrainer()
{
    HANDLE thread = NULL;
    DWORD id = 0;
    LONG criada = ExCreateThread(&thread, 0, &id, (PVOID)XapiThreadStartup,
                                 (LPTHREAD_START_ROUTINE)Carrega, NULL, EX_CREATE_FLAG_SYSTEM);
    if (criada < 0 || thread == NULL)
        return criada < 0 ? criada : (LONG)0xC0000001;
    XSetThreadProcessor(thread, 4);
    SetThreadPriority(thread, THREAD_PRIORITY_TIME_CRITICAL);
    ResumeThread(thread);
    CloseHandle(thread);
    // espera sem limite, como o original: a thread roda código deste loader,
    // então o loader não pode fechar antes de ela terminar
    while (!g_terminou)
        Sleep(60);
    return g_status;
}

void __cdecl main()
{
    HANDLE modulo = NULL;
    // carregar duas vezes criaria duas threads lendo o controle
    if (XexGetModuleHandle(NOME_TRAINER, &modulo) >= 0 && modulo != NULL) {
        Notifica(L"Trainer RE5 TU5 by @lux-insider já está carregado. Abra o jogo e aperte CIMA + START.");
        Sleep(3000);
        return;
    }
    // não tenta carregar por cima de outro módulo
    if (!AreaLivre()) {
        WCHAR texto[256];
        AreasLivres(texto, 256, swprintf_s(texto, 256, L"Trainer RE5: a área %08X está em uso. Livres:", BASE_TRAINER));
        Notifica(texto);
        Sleep(6000);
        return;
    }
    LONG status = CarregaTrainer();
    if (status < 0) {
        WCHAR texto[256];
        AreasLivres(texto, 256, swprintf_s(texto, 256, L"Trainer RE5: erro %08X. Livres:", (DWORD)status));
        Notifica(texto);
        Sleep(6000);
        return;
    }
    Notifica(L"Trainer RE5 TU5 by @lux-insider carregado. Abra o jogo e aperte CIMA + START.");
    Sleep(3000);
    // ao sair do main, o console volta ao painel (Aurora)
}
