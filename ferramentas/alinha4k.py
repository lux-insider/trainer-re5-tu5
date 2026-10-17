"""Marca um PE do Xbox 360 como alinhado em 4 KB (SectionAlignment 0x1000).
O linker do XDK sempre grava 64 KB. As seções continuam nos mesmos endereços;
o imagexex confere depois se seções de atributos diferentes ficaram na mesma
página (erro "sections with different attributes combined onto one page").
Com isso o imagexex aceita a região de páginas de 4 KB (0x90000000-0x91FEFFFF),
onde ficam os plugins do console. uso: alinha4k.py arquivo.dll"""
import struct, sys
p = sys.argv[1]; d = bytearray(open(p, 'rb').read())
pe = struct.unpack('<I', d[0x3C:0x40])[0]; o = pe + 24
antes = struct.unpack('<I', d[o + 32:o + 36])[0]
assert antes in (0x1000, 0x10000), 'alinhamento inesperado: %X' % antes
struct.pack_into('<I', d, o + 32, 0x1000)
open(p, 'wb').write(d)
print('SectionAlignment %X -> 1000' % antes)
