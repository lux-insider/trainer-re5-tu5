# Trainer RE5 TU5 by @lux-insider

Trainer para o **Resident Evil 5** de Xbox 360 com a **TU5**, para console
desbloqueado (RGH/JTAG). Feito do zero, com conferências para não travar o
jogo: antes de gravar qualquer coisa ele confere o jogo aberto e as
instruções que vai trocar.

## Trapaças

Na partida, segure **CIMA (direcional) + START** por 1 ou 2 segundos.
Aparece o aviso "Trainer RE5: abrindo..." e abre uma caixa onde você digita
um código de 4 botões.

| Código | Trapaça |
|---|---|
| RT RT RT RT | Vida infinita |
| LT LT LT LT | Munição infinita |
| RB RB RB RB | Granadas infinitas |
| X X X X | Minas infinitas |
| LB LB LB LB | Dinheiro infinito |
| Y Y Y Y | Pontos do Mercenaries |
| LB RB LB RB | Pontos das DLCs |
| LT RT LT RT | Relógio parado |
| CIMA ×4 | Liga ou desliga todas |
| BAIXO ×4 | Lista dos códigos, com as que estão ligadas |

O mesmo código liga e desliga. Depois aparece o aviso "...: LIGADA" ou
"DESLIGADA", e o controle vibra: **uma vibração longa = ligada**, **duas
curtas = desligada**. B fecha a caixa sem fazer nada.

## Requisitos

- Xbox 360 desbloqueado (RGH/JTAG) com DashLaunch.
- Resident Evil 5 na **TU5** (Title ID `434307D4`).
- Para os avisos no canto da tela, o **JRPC2** carregado como plugin do
  DashLaunch. Sem ele as trapaças funcionam igual e a vibração mostra o
  estado.

## Como usar

1. Baixe o `.zip` da página de [Releases](../../releases) e copie os dois
   arquivos para a mesma pasta no console, por exemplo
   `Usb:\Apps\RE5 Trainer TU5\` ou `Hdd1:\Trainers\RE5\`.
2. No Aurora, abra o `RE5 Trainer Loader.xex` (pelo gerenciador de
   arquivos, por exemplo). Aparece "Trainer RE5 TU5 by @lux-insider
   carregado" e o console volta ao Aurora.
3. Abra o Resident Evil 5 sem desligar o console. Ao entrar no jogo aparece
   "Trainer RE5 by @lux-insider ativo".
4. Na partida, CIMA + START e digite o código.

O trainer fica na memória até o console desligar. Para trocar por uma
versão nova, tire o console da tomada antes de abrir o loader de novo.

## Segurança

- Só age com o Resident Evil 5 aberto. No Aurora ou em outro jogo, o
  CIMA + START não faz nada.
- Antes de gravar, confere que as 12 instruções do jogo estão exatamente
  como na TU5. Se uma não bater, não grava nada e avisa.
- O estado de cada trapaça é lido da própria memória do jogo, então nunca
  fica invertido. Cada gravação é conferida lendo de volta; se uma falhar,
  a trapaça volta ao original.
- As gravações são protegidas: se a memória recusar, o trainer avisa em vez
  de derrubar o jogo.
- Não mexe em arquivos, saves, rede, NAND nem kv.bin.
- Use jogando sozinho ou offline.

## Compilar

Precisa do Xbox 360 XDK 21256 e do Wine (o script roda o compilador do XDK
pelo Wine).

```sh
XDK=/caminho/para/o/XDK ./compilar.sh
```

Gera `saida/RE5 Trainer Loader.xex` e `saida/re5trainer.xex`.

O trainer é uma DLL de sistema em `0x91500000`, uma área livre da região de
plugins. O script marca o PE com páginas de 4 KB
(`ferramentas/alinha4k.py`) para o `imagexex` aceitar essa região e ajusta
as permissões da imagem (`ferramentas/flags_imagem.py`).

## Licença

[MIT](LICENSE). Resident Evil é marca da CAPCOM; este é um projeto de fã,
gratuito e sem fins lucrativos.
