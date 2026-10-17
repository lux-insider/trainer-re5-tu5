"""Grava as permissões (image flags) do XEX igual às do trainer original.
uso: flags_imagem.py ARQUIVO.xex VALOR_HEX
Os image flags ficam na parte assinada do cabeçalho de segurança, fora do hash
do cabeçalho; o console com RGH/JTAG não confere a assinatura."""
import struct, sys
caminho, valor = sys.argv[1], int(sys.argv[2], 16)
d = bytearray(open(caminho, 'rb').read())
assert d[:4] == b'XEX2'
sec, = struct.unpack('>I', d[16:20])
antes, = struct.unpack('>I', d[sec + 0x10C:sec + 0x110])
# mantém o bit de páginas de 4 KB que o imagexex gravou
assert antes & ~0x10000000 == 0, 'image flags inesperados: %08X' % antes
struct.pack_into('>I', d, sec + 0x10C, valor | antes)
open(caminho, 'wb').write(d)
print('image flags %08X -> %08X' % (antes, valor | antes))
