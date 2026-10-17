"""Cabeçalho de um XEX2 do Xbox 360 (só leitura). uso: xex_info.py ARQUIVO"""
import struct, sys
d = open(sys.argv[1], 'rb').read()
mag, flags, pe_off, _, sec_off, nopt = struct.unpack('>4sIIIII', d[:24])
print('magic', mag, 'module_flags %08X' % flags, 'pe_data_offset %X' % pe_off, 'security %X' % sec_off, 'opt headers', nopt)
NOMES = {0x2FF: 'resource_info?', 0x3FF: 'file_format_info', 0x5FF: 'delta_patch', 0x4004: 'bounding_path', 0x80FF: 'device_id',
         0x10001: 'original_base_address', 0x10100: 'entry_point', 0x10201: 'image_base_address', 0x103FF: 'import_libraries',
         0x18002: 'checksum_timestamp', 0x183FF: 'original_pe_name', 0x200FF: 'static_libraries', 0x20104: 'tls_info',
         0x20200: 'default_stack_size', 0x20301: 'default_fs_cache', 0x20401: 'default_heap_size', 0x40006: 'execution_info',
         0x40310: 'game_ratings', 0x40404: 'lan_key', 0x30000: 'system_flags', 0x40201: 'alt_title_ids'}
for k in range(nopt):
    key, val = struct.unpack('>II', d[24 + k * 8: 32 + k * 8])
    nome = NOMES.get(key, '?')
    info = ''
    if key == 0x3FF:
        tam, enc, comp = struct.unpack('>IHH', d[val:val + 8])
        info = 'encryption=%d compression=%d' % (enc, comp)
    elif key == 0x40006:
        mid, ver, bver, tid = struct.unpack('>IIII', d[val:val + 16])
        info = 'media_id %08X version %08X title_id %08X' % (mid, ver, tid)
    elif key == 0x183FF:
        n = struct.unpack('>I', d[val:val + 4])[0]; info = d[val + 4:val + n].split(b'\0')[0].decode('latin1')
    elif key == 0x103FF:
        n = struct.unpack('>I', d[val:val + 4])[0]; blob = d[val:val + n]
        info = sorted(set(s.decode() for s in __import__('re').findall(rb'[a-z]+\.xex', blob)))
    elif key in (0x10100, 0x10201, 0x10001, 0x20200):
        info = '%08X' % val
    print('  %06X %-22s %s' % (key, nome, info))
img_size, = struct.unpack('>I', d[sec_off + 4:sec_off + 8])
load_addr, = struct.unpack('>I', d[sec_off + 0x110:sec_off + 0x114])
print('image_size %X load_address %X' % (img_size, load_addr))
