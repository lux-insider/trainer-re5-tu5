# Como contribuir

Obrigado pelo interesse! Este é um projeto pessoal e gratuito, mantido no
tempo livre. Toda ajuda é bem-vinda: relatar erros, sugerir ideias ou enviar
código.

## Relatar erros e sugerir ideias

Use as [issues](https://github.com/lux-insider/trainer-re5-tu5/issues/new/choose): há um
modelo para erros e outro para ideias. Para falhas de segurança, veja o
[SECURITY.md](SECURITY.md).

## Compilar e testar

O trainer é compilado com o XDK da Microsoft (no Linux, pelo Wine). O passo a
passo está na seção [Compilação](README.md#compilação) do README:

```
./compilar.sh
```

Teste sempre no console antes de abrir o pull request: o trainer mexe na
memória do jogo e um endereço errado trava o console.

## Enviar uma mudança

1. Crie um branch a partir da `main`.
2. Faça a mudança com testes, quando der.
3. Atualize o [CHANGELOG.md](CHANGELOG.md) se a mudança aparece para quem usa.
4. Abra o pull request e preencha o modelo.

## Estilo

- Textos do programa, comentários e documentação em **português**.
- Fontes em UTF-8 **com BOM** (sem o BOM os acentos saem errados no XDK).
- Commits curtos e no imperativo, em português (ex.: "Corrige o checksum do Xbox").

## Licença

Ao contribuir, você concorda que o seu código seja distribuído sob a
[licença MIT](LICENSE) do projeto.
