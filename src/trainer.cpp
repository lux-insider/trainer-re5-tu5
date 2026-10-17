// Trainer do Resident Evil 5 TU5 para Xbox 360 (DLL de sistema, re5trainer.xex).
//
// O loader carrega esta DLL e volta ao Aurora. Ela fica esperando, no
// controle 1, CIMA (direcional) + START com o RE5 aberto, como o trainer original; então abre a caixa de código
// do sistema (4 botões) e liga ou desliga a trapaça escolhida.
//
// Segurança: antes de gravar qualquer coisa, o trainer confere que
//   1. o jogo aberto é o Resident Evil 5 (Title ID 434307D4);
//   2. as 12 instruções conhecidas estão exatamente como na TU5 (a original
//      ou o nop). Se uma não bater, nada é gravado.
// O estado de cada trapaça é lido da memória do jogo, então nunca fica
// invertido; cada gravação é conferida lendo de volta. Fora do RE5 o trainer
// não lê nem grava nada. Não mexe em arquivos, saves, rede, NAND ou kv.bin.
#include "xdk_extra.h"
#include "trapacas.h"

static const DWORD TITLE_ID_RE5 = 0x434307D4;
static const DWORD CONTROLE = 0;        // controle 1
// espera entre tentativas de abrir uma caixa: logo depois de fechar uma, o
// sistema recusa a próxima (acesso negado) até terminar de fechar
static const DWORD ESPERA_MS = 100;
static const int TENTATIVAS_CAIXA = 30;
// espera a caixa sumir da tela antes de avisar; avisos enviados enquanto ela
// fecha são descartados pelo sistema
static const DWORD AVISO_MS = 2000;
// intervalo de leitura do controle, igual ao trainer original: ler o controle
// muito seguido enquanto o jogo carrega a partida travava o RE5
static const DWORD LEITURA_MS = 500;

static PCWSTR const TITULO = L"Trainer RE5 TU5 by @lux-insider";

// Diagnóstico lido pela rede (xbdm getmem): o que a thread viu e onde parou.
struct Diagnostico {
    DWORD marca;        // 'DIAG'
    DWORD voltas;       // voltas do laço
    DWORD titulo;       // último Title ID
    DWORD leitura;      // retorno do XamInputGetState
    DWORD botoes;       // últimos botões lidos
    DWORD combos;       // vezes que CIMA + START foi detectado
    DWORD etapa;        // 2 jogo 3 erro caixa 4 caixa aberta 5 caixa fechou 6 executou
    DWORD erro;         // código da caixa
    DWORD achou;        // ao carregar: 4 notificação
    DWORD falhas;       // leituras ou gravações recusadas pela memória
};
static volatile Diagnostico g_diag = { 0x44494147 };

// ---------------------------------------------------------------- memória do jogo

// Lê e grava direto na memória do jogo, como os plugins de console
// desbloqueado. O xbdm deste console é uma versão reduzida que não oferece
// funções para outros programas, então não dá para usar o DmSetMemory.
// Cada acesso fica protegido: se a memória recusar, devolve FALSE em vez de
// derrubar o jogo.
static BOOL Le(DWORD endereco, DWORD* valor)
{
    if (!MmIsAddressValid((PVOID)endereco))
        return FALSE;
    __try {
        *valor = *(volatile DWORD*)endereco;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        g_diag.falhas++;
        return FALSE;
    }
    return TRUE;
}

// Grava 4 bytes, avisa o processador que o código mudou e confere lendo de volta.
static BOOL Grava(DWORD endereco, DWORD valor)
{
    DWORD conferido = 0;
    if (!MmIsAddressValid((PVOID)endereco))
        return FALSE;
    __try {
        *(volatile DWORD*)endereco = valor;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        g_diag.falhas++;
        return FALSE;
    }
    // a instrução está no cache: grava na memória e descarta a cópia antiga
    KeSweepDcacheRange((PVOID)endereco, sizeof(DWORD));
    KeSweepIcacheRange((PVOID)endereco, sizeof(DWORD));
    return Le(endereco, &conferido) && conferido == valor;
}

// ---------------------------------------------------------------- conferências

static BOOL Re5Aberto()
{
    return XamGetCurrentTitleId() == TITLE_ID_RE5;
}

// Todas as instruções conhecidas estão como na TU5 (originais ou nop)?
static BOOL JogoConfere()
{
    for (int t = 0; t < NUM_TRAPACAS; t++) {
        for (int i = 0; i < TRAPACAS[t].quantas; i++) {
            const Instrucao& ins = TRAPACAS[t].instrucoes[i];
            DWORD valor = 0;
            if (!Le(ins.endereco, &valor))
                return FALSE;
            if (valor != ins.original && valor != INSTRUCAO_NOP)
                return FALSE;
        }
    }
    return TRUE;
}

// Ligada = todas as instruções da trapaça são nop.
static BOOL Ligada(const Trapaca& t)
{
    for (int i = 0; i < t.quantas; i++) {
        DWORD valor = 0;
        if (!Le(t.instrucoes[i].endereco, &valor) || valor != INSTRUCAO_NOP)
            return FALSE;
    }
    return TRUE;
}

// ---------------------------------------------------------------- ligar e desligar

// Liga (nop) ou desliga (original). Se uma gravação falhar, devolve a
// trapaça inteira ao original.
static BOOL Aplica(const Trapaca& t, BOOL ligar)
{
    for (int i = 0; i < t.quantas; i++) {
        const Instrucao& ins = t.instrucoes[i];
        if (!Grava(ins.endereco, ligar ? INSTRUCAO_NOP : ins.original)) {
            for (int j = 0; j < t.quantas; j++)
                Grava(t.instrucoes[j].endereco, t.instrucoes[j].original);
            return FALSE;
        }
    }
    return TRUE;
}

// Vibração que também diz o estado: uma longa = ligada, duas curtas = desligada.
static VOID Pulso(DWORD ms)
{
    XINPUT_VIBRATION v;
    v.wLeftMotorSpeed = 20000;
    v.wRightMotorSpeed = 20000;
    XInputSetState(CONTROLE, &v);
    Sleep(ms);
    v.wLeftMotorSpeed = 0;
    v.wRightMotorSpeed = 0;
    XInputSetState(CONTROLE, &v);
}

static VOID Vibra(BOOL ligada)
{
    if (ligada) {
        Pulso(600);
    } else {
        Pulso(150);
        Sleep(150);
        Pulso(150);
    }
}

static VOID AvisaEstado(PCWSTR nome, BOOL ligada)
{
    WCHAR texto[96];
    swprintf_s(texto, 96, L"%s: %s", nome, ligada ? L"LIGADA" : L"DESLIGADA");
    Notifica(texto);
}

// ---------------------------------------------------------------- caixas

// Resultado e controle da caixa em memória fixa, como no trainer original: o
// sistema escreve neles depois que a função já voltou, então não podem ficar
// na pilha.
static MESSAGEBOX_RESULT g_resultado;
static XOVERLAPPED g_ov;

// Mostra uma caixa do sistema e espera fechar. No modo senha, devolve os 4
// botões em codigo. Devolve FALSE se foi cancelada (B) ou não abriu.
static BOOL MostraCaixa(PCWSTR texto, DWORD estilo, WORD codigo[4])
{
    LPCWSTR botoes[1] = { L"Fechar" };
    ZeroMemory(&g_resultado, sizeof(g_resultado));
    ZeroMemory(&g_ov, sizeof(g_ov));
    // o sistema avisa por este evento quando a caixa fecha
    g_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ov.hEvent == NULL)
        return FALSE;
    DWORD aberta = ERROR_ACCESS_DENIED;
    for (int t = 0; t < TENTATIVAS_CAIXA && aberta == ERROR_ACCESS_DENIED; t++) {
        if (t > 0)
            Sleep(ESPERA_MS);
        aberta = XShowMessageBoxUI(CONTROLE, TITULO, texto, 1, botoes, 0, estilo, &g_resultado, &g_ov);
    }
    g_diag.erro = aberta;
    if (aberta != ERROR_IO_PENDING) {
        g_diag.etapa = 3;
        CloseHandle(g_ov.hEvent);
        WCHAR aviso[96];
        swprintf_s(aviso, 96, L"Trainer RE5: não consegui abrir a caixa (erro %08X).", aberta);
        Notifica(aviso);
        return FALSE;
    }
    g_diag.etapa = 4;
    // espera a caixa fechar (no máximo 2 minutos); se passar, cancela e espera
    // o sistema confirmar, para ele não escrever depois
    if (WaitForSingleObject(g_ov.hEvent, 120000) != WAIT_OBJECT_0) {
        XCancelOverlapped(&g_ov);
        WaitForSingleObject(g_ov.hEvent, INFINITE);
        CloseHandle(g_ov.hEvent);
        return FALSE;
    }
    DWORD resultado = 0;
    DWORD final = XGetOverlappedResult(&g_ov, &resultado, FALSE);
    CloseHandle(g_ov.hEvent);
    g_diag.etapa = 5;
    g_diag.erro = final;
    if (final != ERROR_SUCCESS)         // B fecha a caixa (cancelada)
        return FALSE;
    if (codigo != NULL)
        for (int i = 0; i < 4; i++)
            codigo[i] = g_resultado.rgwPasscode[i];
    return TRUE;
}

static BOOL Igual(const WORD a[4], const WORD b[4])
{
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static VOID MostraLista()
{
    WCHAR texto[1024];
    int n = swprintf_s(texto, 1024, L"CIMA + START abre esta caixa; digite 4 botões.\n\n");
    for (int t = 0; t < NUM_TRAPACAS && n > 0; t++)
        n += swprintf_s(texto + n, 1024 - n, L"%s  %s%s\n", TRAPACAS[t].botoes, TRAPACAS[t].nome,
                        Ligada(TRAPACAS[t]) ? L"  (ligada)" : L"");
    if (n > 0)
        swprintf_s(texto + n, 1024 - n, L"CIMA x4  Todas\nBAIXO x4  Esta lista");
    MostraCaixa(texto, XMB_NOICON, NULL);
}

// ---------------------------------------------------------------- código digitado

static VOID Executa(const WORD codigo[4])
{
    if (codigo[0] == 0)             // fechou a caixa (A) sem digitar código
        return;
    if (Igual(codigo, CODIGO_LISTA)) {
        MostraLista();
        return;
    }
    Sleep(AVISO_MS);
    // confere de novo: o jogo pode ter mudado enquanto a caixa estava aberta
    if (!Re5Aberto() || !JogoConfere()) {
        Notifica(L"Trainer RE5: o jogo não confere. Nada foi alterado.");
        return;
    }
    if (Igual(codigo, CODIGO_TODAS)) {
        BOOL todasLigadas = TRUE;
        for (int t = 0; t < NUM_TRAPACAS; t++)
            if (!Ligada(TRAPACAS[t]))
                todasLigadas = FALSE;
        BOOL ok = TRUE;
        for (int t = 0; t < NUM_TRAPACAS; t++)
            if (!Aplica(TRAPACAS[t], !todasLigadas))
                ok = FALSE;
        if (!ok)
            Notifica(L"Trainer RE5: uma gravação falhou e foi desfeita.");
        else
            AvisaEstado(L"Todas as trapaças", !todasLigadas);
        Vibra(ok && !todasLigadas);
        return;
    }
    for (int t = 0; t < NUM_TRAPACAS; t++) {
        if (Igual(codigo, TRAPACAS[t].codigo)) {
            BOOL ligar = !Ligada(TRAPACAS[t]);
            BOOL ok = Aplica(TRAPACAS[t], ligar);
            if (ok)
                AvisaEstado(TRAPACAS[t].nome, ligar);
            else
                Notifica(L"Trainer RE5: a gravação falhou e foi desfeita.");
            Vibra(ok && ligar);
            return;
        }
    }
    Notifica(L"Trainer RE5: código desconhecido. BAIXO x4 mostra a lista.");
}

// Abre a caixa de código, depois de conferir tudo.
static VOID AbreTrainer()
{
    g_diag.combos++;
    if (!JogoConfere()) {
        g_diag.etapa = 2;
        Notifica(L"Trainer RE5: versão do jogo não reconhecida (precisa da TU5). Nada foi alterado.");
        return;
    }
    Notifica(L"Trainer RE5: abrindo...");
    Sleep(1000);                    // deixa o aviso aparecer antes da caixa
    WORD codigo[4];
    BOOL ok = MostraCaixa(L"Digite o código de 4 botões.\nBAIXO x4 mostra a lista.", XMB_NOICON | XMB_PASSCODEMODE, codigo);
    if (ok) {
        g_diag.etapa = 6;
        Executa(codigo);
    }
}

// ---------------------------------------------------------------- thread principal

static DWORD WINAPI Principal(PVOID)
{
    const WORD COMBINACAO = XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_START;
    BOOL apertadaAntes = TRUE;      // exige soltar antes da primeira vez
    BOOL avisou = FALSE;            // avisou que o RE5 foi detectado?

    for (;;) {
        Sleep(LEITURA_MS);
        g_diag.voltas++;
        g_diag.titulo = XamGetCurrentTitleId();
        if (!Re5Aberto()) {         // fora do RE5 não faz nada
            apertadaAntes = TRUE;
            avisou = FALSE;
            continue;
        }
        if (!avisou) {
            Notifica(L"Trainer RE5 by @lux-insider ativo. CIMA + START abre o trainer.");
            avisou = TRUE;
        }
        XINPUT_STATE estado;
        ZeroMemory(&estado, sizeof(estado));
        DWORD leitura = XamInputGetState(CONTROLE, 1, &estado);
        g_diag.leitura = leitura;
        g_diag.botoes = estado.Gamepad.wButtons;
        if (leitura != ERROR_SUCCESS) {
            apertadaAntes = TRUE;
            continue;
        }
        BOOL apertada = (estado.Gamepad.wButtons & COMBINACAO) == COMBINACAO;
        if (apertada && !apertadaAntes)
            AbreTrainer();
        // só abre de novo depois de soltar CIMA + START
        apertadaAntes = apertada;
    }
}

BOOL APIENTRY DllMain(HANDLE, DWORD motivo, LPVOID)
{
    if (motivo == DLL_PROCESS_ATTACH) {
        // procura a notificação agora, ainda no Aurora, como o trainer original
        ResolveNotifica();
        g_diag.achou = g_notifica ? 4 : 0;
        HANDLE thread = NULL;
        DWORD id = 0;
        // thread de sistema, como a do trainer original
        if (ExCreateThread(&thread, 0, &id, (PVOID)XapiThreadStartup,
                           (LPTHREAD_START_ROUTINE)Principal, NULL, EX_CREATE_FLAG_SYSTEM) < 0)
            return FALSE;
        if (thread != NULL) {
            // núcleo 4 e prioridade normal, como o trainer original (prioridade
            // alta deixava o jogo lento)
            XSetThreadProcessor(thread, 4);
            ResumeThread(thread);
            CloseHandle(thread);
        }
    }
    return TRUE;
}
