#!/bin/bash
# Compila o Trainer RE5 TU5 pelo Wine com o XDK 21256.
# A pasta do XDK vem da variável XDK (padrão: ~/xdk/extraido/XDK).
# Gera em saida/: "RE5 Trainer Loader.xex" e re5trainer.xex.
set -e
cd "$(dirname "$0")"
XDK=$(realpath "${XDK:-$HOME/xdk/extraido/XDK}")
J=$XDK/TechPreview/Jul12Compiler          # compilador com os cabeçalhos completos da CRT
W_XDK="Z:${XDK//\//\\}"
W_J="$W_XDK"'\TechPreview\Jul12Compiler'
export WINEDEBUG=-all
export INCLUDE="$W_J\\include\\xbox;$W_XDK\\include\\xbox"
export LIB="$W_J\\lib\\xbox;$W_XDK\\lib\\xbox"
export WINEPATH="$W_J\\bin\\win32;$W_XDK\\bin\\win32"
encerra() { wineserver -k 2>/dev/null || true; }   # o mspdbsrvX fica rodando e prenderia o terminal
trap encerra EXIT

rm -rf obj saida && mkdir -p obj saida
CL="timeout 180 wine $J/bin/win32/cl.exe /nologo /c /O2 /W4 /WX /GS /DNDEBUG /DNOD3D /DNONET /Isrc"
LINK="timeout 180 wine $J/bin/win32/link.exe /nologo /XEX:NO /DEBUG:NONE"
IMAGEXEX="timeout 180 wine $XDK/bin/win32/imagexex.exe /nologo"
LIBS="xapilib.lib xboxkrnl.lib libcmt.lib"

# trainer: DLL de sistema em 0x91500000 (área livre no console; 0x91240000 já é usado por outro módulo)
$CL /Foobj/trainer.obj src/trainer.cpp </dev/null
$LINK /DLL /BASE:0x91500000 /OUT:obj/re5trainer.dll obj/trainer.obj $LIBS </dev/null
python3 -I ferramentas/alinha4k.py obj/re5trainer.dll
$IMAGEXEX /CONFIG:xex/trainer.xml /IN:obj/re5trainer.dll /OUT:saida/re5trainer.xex </dev/null
# permissões iguais às do trainer original (sem elas o console nega o carregamento: C0000022)
python3 -I ferramentas/flags_imagem.py saida/re5trainer.xex 30000006
# o loader confere a área 0x91500000-0x91530000 antes de carregar: a imagem tem de caber nela
python3 -I - saida/re5trainer.xex <<'PY'
import struct, sys
d = open(sys.argv[1], 'rb').read()
sec, = struct.unpack('>I', d[16:20])
tam, = struct.unpack('>I', d[sec + 4:sec + 8]); base, = struct.unpack('>I', d[sec + 0x110:sec + 0x114])
assert base == 0x91500000 and tam <= 0x30000, 'trainer em %08X com %X bytes não cabe na área do loader' % (base, tam)
print('trainer em %08X-%08X' % (base, base + tam))
PY

# loader: aplicativo comum, aberto pelo Aurora
$CL /Foobj/loader.obj src/loader.cpp </dev/null
$LINK /OUT:obj/loader.exe obj/loader.obj $LIBS </dev/null
$IMAGEXEX /IN:obj/loader.exe "/OUT:saida/RE5 Trainer Loader.xex" </dev/null

ls -la saida
