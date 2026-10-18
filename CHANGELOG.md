# Histórico de versões

| Versão | Data | Destaque |
|---|---|---|
| [1.0](#10--2026-10-17) | 17/10/2026 | Primeira versão |

---

## 1.0 — 2026-10-17

Primeira versão.

### Adicionado
- Caixa de código aberta com **CIMA + START** durante a partida.
- 8 trapaças: vida, munição, granadas, minas e dinheiro infinitos, pontos do
  Mercenaries, pontos das DLCs e relógio parado.
- **CIMA ×4** liga ou desliga todas; **BAIXO ×4** mostra a lista de códigos
  com o estado de cada trapaça.
- Aviso de LIGADA/DESLIGADA no canto da tela (com o JRPC2) e confirmação por
  vibração: uma longa = ligada, duas curtas = desligada.

### Segurança
- Só age com o Resident Evil 5 aberto (Title ID `434307D4`).
- Confere as 12 instruções da TU5 antes de gravar; com outra versão do jogo,
  não grava nada.
- Cada gravação é lida de volta; se falhar, a trapaça volta ao original.
- Gravações protegidas contra falha de acesso à memória.
