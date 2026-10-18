# Como o trainer funciona

Visão técnica do Trainer RE5 TU5: como ele é carregado, como aplica as
trapaças e o que confere antes de mexer na memória do jogo.

## Componentes

| Arquivo | Papel |
|---|---|
| `RE5 Trainer Loader.xex` | aberto pelo Aurora; carrega o trainer e volta ao dashboard |
| `re5trainer.xex` | o trainer: DLL de sistema que fica residente até o console desligar |

## Carregamento

1. O loader confere se a área de memória reservada para o trainer
   (`0x91500000`–`0x91530000`, na região de plugins) está livre.
2. O console só aceita carregar uma DLL de sistema a partir de uma **thread de
   sistema**: carregada pela thread do aplicativo, a chamada é negada
   (`C0000022`). Por isso o loader cria uma thread de sistema
   (`ExCreateThread`), carrega o `re5trainer.xex` nela e espera o fim.
3. Abrir o loader de novo não carrega uma segunda cópia.

A imagem é ligada em `0x91500000` com páginas de 4 KB
(`ferramentas/alinha4k.py`), para o `imagexex` aceitar essa região, e recebe
as permissões de imagem de sistema (`ferramentas/flags_imagem.py`). O
`compilar.sh` confere no fim que a imagem cabe na área reservada.

## Trapaças

Cada trapaça troca **uma ou duas instruções** do executável do jogo
(`default.xex` da TU5) por `nop` (`0x60000000`). A instrução que gravaria o
valor (vida, munição, dinheiro...) deixa de rodar, e o valor fica parado.
Desligar grava a instrução original de volta.

A tabela completa (endereço e instrução original de cada uma das 12
instruções) fica em [`src/trapacas.h`](../src/trapacas.h).

## Conferências

Antes de qualquer gravação, o trainer confere:

1. **o jogo aberto** é o Resident Evil 5 (Title ID `434307D4`);
2. **as instruções** em cada endereço são exatamente as da TU5 (ou o `nop`
   que o próprio trainer gravou). Se uma não bater, nada é gravado e aparece
   "versão do jogo não reconhecida".

Durante a gravação:

- o estado de cada trapaça é lido da própria memória do jogo, então nunca
  fica invertido;
- a gravação é feita dentro de um bloco protegido (`__try`): se a memória
  recusar, o trainer avisa em vez de derrubar o jogo;
- depois de gravar, os caches de dados e de instruções são sincronizados e o
  valor é lido de volta; se não conferir, a trapaça volta ao original.

O trainer não grava em endereços fixos de dados e não mexe em arquivos,
saves, rede, NAND nem kv.bin.

## Leitura do controle

A thread do trainer roda no núcleo 4, lê o controle a cada 500 ms e só reage
com o RE5 aberto. Os avisos usam o sistema de notificações do
console (`XNotify`), que no Aurora depende do plugin JRPC2.

## Mensagens

| Mensagem | Significado |
|---|---|
| versão do jogo não reconhecida | o executável não é a TU5 esperada; nada foi gravado |
| a gravação falhou e foi desfeita | a memória recusou a gravação; nada mudou |
| o jogo não confere | o jogo mudou enquanto a caixa estava aberta |
| código desconhecido | o código digitado não existe; BAIXO ×4 mostra a lista |
