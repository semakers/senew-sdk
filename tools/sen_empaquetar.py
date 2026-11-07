#!/usr/bin/env python3
"""sen_empaquetar.py — del ELF de una app (enlazada en 0 con lib/w5_app.ld y --emit-relocs) al fichero .sen.
   uso: sen_empaquetar.py app.elf app.sen [--rec recursos.bin] [--api MAYOR.MENOR] [--arte icono.txt] [--malla malla.bin]
        sen_empaquetar.py --poner-icono entra.sen sale.sen [--arte icono.txt] [--malla malla.bin] [--nombre «otro nombre»]
   (ICO) EL ICONO: --arte = 12 líneas de la misma longitud (1..16; «#» = tinta, otra cosa = fondo): el icono PLANO de la
   cabecera, el que pintan los dos diseños de la rejilla (si no se da, el de W5_APP). --malla = la sección de la malla 3D
   (W5xIcoMalla + pos + nor + tri, api/cargable.h): se VALIDA aquí igual que en el teléfono y va al final de los recursos con
   su pie «W5I1». --poner-icono cambia el icono de una .sen YA HECHA (quita la sección que tuviera y rehace los CRC) sin
   tocar ni un byte de su código, sus datos ni sus recursos. OJO: sin --malla, la .sen sale SIN malla (así se quita).
   Lo difícil se decide AQUÍ, en el PC: cualquier reubicación que no sea «palabra absoluta» o «relativa al contador
   de programa» hace fallar el empaquetado. El teléfono solo sabe sumar la base a una lista de palabras.
   Formato: cargable.h.
   (1.1) VENTANA DE RECURSOS: si el ELF trae una sección .w5rec enlazada en W5_REC_VA (lib/w5_juego.ld),
   su contenido son los RECURSOS del .sen (no hace falta --rec) y la app tiene que declarar W5_CAP_REC_VENTANA. Lo que
   apunta a la ventana (desde el código, los datos u otros recursos) NO se reubica: es una dirección fija. Y lo que hay
   DENTRO de la ventana no puede apuntar a la imagen (la flash no se reubica): eso hace fallar el empaquetado."""
import struct, sys, zlib
ABS = {40: {2, 38}, 183: {257}}                                  # R_ARM_ABS32, R_ARM_TARGET1 | R_AARCH64_ABS64
REL = {40: {0, 1, 3, 10, 28, 29, 30, 40, 42, 102, 103},          # NONE PC24 REL32 THM_CALL CALL JUMP24 THM_JUMP24 V4BX PREL31 THM_JUMP11 THM_JUMP8
       183: {0, 260, 261, 262, 273, 274, 279, 280, 282, 283}}    # NONE PREL64/32/16 LD_PREL_LO19 ADR_PREL_LO21 TSTBR14 CONDBR19 JUMP26 CALL26
REC_VA, REC_MAX, CAP_REC_VENTANA = 0x70000000, 16 << 20, 1 << 10
def error(m): print('sen_empaquetar: ' + m, file=sys.stderr); sys.exit(1)
ICO_MAGIA, ICO_MAX, ICO_TRI_MAX, ICO_CAB = b'W5I1', 3072, 256, 26
def malla_vale(m):
    """(ICO) las MISMAS reglas que cg_ico_validar (del cargador del teléfono): None si vale; si no, por qué."""
    if not ICO_CAB <= len(m) <= ICO_MAX: return 'la malla mide %d B (de %d a %d)' % (len(m), ICO_CAB, ICO_MAX)
    if m[0] != 1 or m[3] or any(m[23:26]): return 'versión que no es 1 o reservados que no son 0'
    npos, nnor = m[1], m[2]
    nt = struct.unpack_from('<5H', m, 4); ntri = sum(nt)
    if npos < 3 or nnor < 1: return 'pocas posiciones o normales'
    if not 1 <= ntri <= ICO_TRI_MAX: return '%d triángulos (de 1 a %d)' % (ntri, ICO_TRI_MAX)
    if len(m) != ICO_CAB + 3 * npos + 3 * nnor + 6 * ntri: return 'el tamaño no cuadra con los contadores'
    t = ICO_CAB + 3 * npos + 3 * nnor
    for i in range(ntri):
        a = m[t + 6 * i:t + 6 * i + 6]
        if max(a[:3]) >= npos or max(a[3:]) >= nnor: return 'el triángulo %d tiene un índice fuera de rango' % i
    return None
def seccion_icono(m):
    e = malla_vale(m)
    if e: error('malla: ' + e)
    return m + struct.pack('<II', len(m), zlib.crc32(m)) + ICO_MAGIA
def arte_bits(txt):
    filas = [f.rstrip('\r') for f in txt.split('\n') if f.strip()]
    if len(filas) != 12 or not 1 <= len(filas[0]) <= 16 or any(len(f) != len(filas[0]) for f in filas):
        error('el arte del icono son 12 filas de la misma longitud (1..16)')
    an = len(filas[0])
    return an, [sum(1 << (an - 1 - k) for k, ch in enumerate(f) if ch == '#') for f in filas]
def poner_icono(w, arte=None, malla=None, nombre=None):
    """(ICO) cambia el icono de una .sen ya hecha: quita su sección de malla si la tenía, pone la nueva y rehace los CRC."""
    cab = bytearray(w[:128])
    if cab[:4] != b'W5X\x01' or zlib.crc32(bytes(cab[:124])) != struct.unpack_from('<I', cab, 124)[0]: error('no es una .sen sana')
    tam_fich, tam_cod, tam_dat = struct.unpack_from('<III', cab, 20)
    n_reub = struct.unpack_from('<I', cab, 48)[0]
    tam_rec, crc_rec = struct.unpack_from('<II', cab, 112)
    if tam_fich != len(w): error('la .sen está cortada')
    ini = 128 + tam_cod + tam_dat + 4 * n_reub
    rec = w[ini:]
    if len(rec) != tam_rec or (tam_rec and zlib.crc32(rec) != crc_rec): error('los recursos no cuadran con la cabecera')
    if len(rec) >= 12 and rec[-4:] == ICO_MAGIA:                        # ya traía una: fuera
        n = struct.unpack_from('<I', rec, len(rec) - 12)[0]
        rec = rec[:len(rec) - 12 - n]
    if malla is not None: rec += seccion_icono(malla)
    if arte is not None:
        an, bits = arte_bits(arte)
        cab[57] = an; struct.pack_into('<12H', cab, 58, *bits)
    if nombre is not None:
        nb = nombre.encode('utf-8')
        if not 0 < len(nb) < 30: error('el nombre es de 1 a 29 bytes')
        cab[82:112] = nb.ljust(30, b'\0')
    struct.pack_into('<I', cab, 20, ini + len(rec))
    struct.pack_into('<II', cab, 112, len(rec), zlib.crc32(rec) if rec else 0)
    struct.pack_into('<I', cab, 124, zlib.crc32(bytes(cab[:124])))
    return bytes(cab) + w[128:ini] + rec
def empaquetar(elf, rec=b'', api=None, arte=None, malla=None):
    if elf[:4] != b'\x7fELF' or elf[5] != 1: error('no es un ELF little-endian')
    c64 = elf[4] == 2
    if c64: (tipo, maq, _, entrada, _, shoff, _, _, _, _, shentsize, shnum, shstrndx) = struct.unpack_from('<HHIQQQIHHHHHH', elf, 16)
    else:   (tipo, maq, _, entrada, _, shoff, _, _, _, _, shentsize, shnum, shstrndx) = struct.unpack_from('<HHIIIIIHHHHHH', elf, 16)
    if maq not in ABS: error('máquina %d: solo ARM (40) y AArch64 (183)' % maq)
    pal = 8 if c64 else 4
    secs = []
    for i in range(shnum):
        o = shoff + i * shentsize
        if c64: n, t, fl, ad, of, sz, lk, inf, al, es = struct.unpack_from('<IIQQQQIIQQ', elf, o)
        else:   n, t, fl, ad, of, sz, lk, inf, al, es = struct.unpack_from('<IIIIIIIIII', elf, o)
        secs.append(dict(n=n, t=t, fl=fl, ad=ad, of=of, sz=sz, lk=lk, inf=inf, es=es))
    strs = secs[shstrndx]
    def nombre(s): a = strs['of'] + s['n']; return elf[a:elf.index(b'\0', a)].decode()
    for s in secs: s['nombre'] = nombre(s)
    ven = [s for s in secs if s['fl'] & 2 and s['sz'] and REC_VA <= s['ad'] < REC_VA + REC_MAX]   # (1.1) la ventana
    if len(ven) > 1: error('más de una sección en la ventana de recursos')
    if ven and rec: error('una app con ventana de recursos no lleva además --rec')
    aloc = [s for s in secs if s['fl'] & 2 and s['sz'] and s not in ven]   # SHF_ALLOC
    cod = [s for s in aloc if s['t'] == 1 and not s['fl'] & 1]     # PROGBITS sin escritura
    dat = [s for s in aloc if s['t'] == 1 and s['fl'] & 1]
    bss = [s for s in aloc if s['t'] == 8]
    fin_cod = max([s['ad'] + s['sz'] for s in cod] or [0])
    ini_dat = min([s['ad'] for s in dat] or [fin_cod]); fin_dat = max([s['ad'] + s['sz'] for s in dat] or [ini_dat])
    ini_bss = min([s['ad'] for s in bss] or [fin_dat]); fin_bss = max([s['ad'] + s['sz'] for s in bss] or [ini_bss])
    if min(s['ad'] for s in aloc) != 0: error('la app no está enlazada en la dirección 0 (usa lib/w5_app.ld)')
    if not (fin_cod <= ini_dat <= fin_dat <= ini_bss <= fin_bss): error('las secciones no van en el orden código, datos, bss')
    tam_cod = ini_dat if dat else (fin_cod + 15) & ~15
    tam_dat = ((fin_dat - tam_cod) + 15) & ~15 if dat else 0
    if bss and ini_bss != tam_cod + tam_dat: error('el .bss no empieza donde acaban los datos (%#x != %#x)' % (ini_bss, tam_cod + tam_dat))
    tam_bss = (fin_bss - ini_bss + 15) & ~15 if bss else 0
    img = bytearray(tam_cod + tam_dat)
    for s in cod + dat: img[s['ad']:s['ad'] + s['sz']] = elf[s['of']:s['of'] + s['sz']]
    total = tam_cod + tam_dat + tam_bss
    # símbolos (para saber si una reubicación absoluta va contra un símbolo absoluto o sin definir: esas no se mueven)
    sims = {}
    for i, s in enumerate(secs):
        if s['t'] == 2:
            k = []
            for j in range(s['sz'] // s['es']):
                o = s['of'] + j * s['es']
                if c64: _, _, _, shndx, val, _ = struct.unpack_from('<IBBHQQ', elf, o)
                else:   _, val, _, _, _, shndx = struct.unpack_from('<IIIBBH', elf, o)
                k.append((shndx, val))
            sims[i] = k
    reub, thumb = set(), bool(entrada & 1)
    v_ini = ven[0]['ad'] if ven else 0
    v_fin = v_ini + ven[0]['sz'] if ven else 0
    if ven:
        if v_ini != REC_VA or ven[0]['t'] != 1: error('la ventana tiene que empezar en %#x y tener contenido' % REC_VA)
        rec = elf[ven[0]['of']:ven[0]['of'] + ven[0]['sz']]
        n_ven = 0
        for s in secs:                                             # lo de DENTRO de la ventana solo puede apuntar a la ventana
            if s['t'] not in (4, 9) or secs[s['inf']] is not ven[0]: continue
            for j in range(s['sz'] // s['es']):
                off, info = struct.unpack_from('<II', elf, s['of'] + j * s['es']); t, si = info & 0xFF, info >> 8
                if t in REL[maq] and t not in (3, 42): continue
                if t not in ABS[maq]: error('reubicación de tipo %d dentro de la ventana (%#x)' % (t, off))
                v = int.from_bytes(elf[ven[0]['of'] + off - v_ini:ven[0]['of'] + off - v_ini + 4], 'little')
                if not v_ini <= v <= v_fin: error('la ventana de recursos apunta FUERA de ella en %#x (-> %#x): la flash no se reubica' % (off, v))
                n_ven += 1
    for s in secs:
        if s['t'] not in (4, 9): continue                          # RELA, REL
        dest = secs[s['inf']]
        if not dest['fl'] & 2 or dest['t'] != 1 or (ven and dest is ven[0]): continue          # solo lo que va a la imagen
        for j in range(s['sz'] // s['es']):
            o = s['of'] + j * s['es']
            if c64: off, info = struct.unpack_from('<QQ', elf, o); t, si = info & 0xFFFFFFFF, info >> 32
            else:   off, info = struct.unpack_from('<II', elf, o); t, si = info & 0xFF, info >> 8
            if t in REL[maq]: continue
            if t not in ABS[maq]:
                error('reubicación de tipo %d en %s+%#x: no se puede llevar a un .sen%s' % (t, dest['nombre'], off,
                      ' (compila con -mcmodel=tiny -fno-pic)' if maq == 183 else ' (compila sin -fpic y para ARMv5)'))
            shndx, val = sims[s['lk']][si]
            if shndx in (0, 0xFFF1): continue                      # sin definir (débil) o absoluto: no se mueve
            if off % pal or off + pal > len(img): error('reubicación sin alinear o fuera de la imagen: %s+%#x' % (dest['nombre'], off))
            v = int.from_bytes(img[off:off + pal], 'little')
            if ven and v_ini <= v <= v_fin: continue               # a la ventana: dirección fija, no se mueve
            if v > total: error('la palabra en %#x apunta fuera de la app (%#x)' % (off, v))
            reub.add(off)
    reub = sorted(reub)
    # los datos de la app: la sección .w5meta
    m = [s for s in secs if s['nombre'] == '.w5meta']
    if not m or m[0]['sz'] < 268: error('falta W5_APP(...) en la app (sección .w5meta)')
    b = elf[m[0]['of']:m[0]['of'] + m[0]['sz']]
    if b[:7] != b'W5META1': error('.w5meta no empieza por W5META1')
    nom = b[8:38].split(b'\0')[0]
    perfil, icono_gen, mayor, menor, an_min, al_min, caps, pila, monton, ver = struct.unpack_from('<BBHHHHIIII', b, 38)
    if api: mayor, menor = api
    if ven and not caps & CAP_REC_VENTANA: error('hay ventana de recursos pero la app no declara W5_CAP_REC_VENTANA')
    if caps & CAP_REC_VENTANA and not ven: error('la app declara W5_CAP_REC_VENTANA pero no hay ventana (¿lib/w5_juego.ld?)')
    if caps & CAP_REC_VENTANA and (mayor, menor) < (1, 1): error('la ventana de recursos es de la API 1.1')
    filas = [b[64 + 17 * i:64 + 17 * i + 17].split(b'\0')[0].decode() for i in range(12)]
    an = len(filas[0])
    if an and any(len(f) != an for f in filas): error('el icono necesita 12 filas de la misma longitud')
    bits = [sum(1 << (an - 1 - k) for k, ch in enumerate(f) if ch == '#') for f in filas] if an else [0] * 12
    if arte is not None: an, bits = arte_bits(arte)                 # (ICO)
    if malla is not None: rec = rec + seccion_icono(malla)           # (ICO) al final de los recursos, con su pie
    pila = (pila + 15) & ~15; monton = (monton + 15) & ~15
    cuerpo = bytes(img) + b''.join(struct.pack('<I', r) for r in reub)
    tam_fich = 128 + len(cuerpo) + len(rec)
    cab = struct.pack('<4sHBBHHIHHIIIIIIIIIBB12H30sII', b'W5X\x01', 128, 1 if maq == 40 else 2, perfil, mayor, menor, caps, an_min, al_min,
                      tam_fich, tam_cod, tam_dat, tam_bss, pila, monton, entrada, len(reub), ver, icono_gen, an, *bits,
                      nom, len(rec), zlib.crc32(rec) if rec else 0)
    cab += struct.pack('<I', zlib.crc32(cuerpo))
    cab += struct.pack('<I', zlib.crc32(cab))
    assert len(cab) == 128
    info = dict(nombre=nom.decode(), maquina='ARMv5TE' if maq == 40 else 'AArch64', thumb=thumb, perfil=perfil, api='%d.%d' % (mayor, menor),
                cod=tam_cod, dat=tam_dat, bss=tam_bss, pila=pila, monton=monton, reub=len(reub), rec=len(rec), fich=tam_fich,
                ram=256 + ((tam_cod + tam_dat + tam_bss + 15) & ~15) + monton + 64 + pila + 256)
    return cab + cuerpo + rec, info
if __name__ == '__main__':
    a = sys.argv[1:]
    if len(a) < 2: print(__doc__); sys.exit(1)
    arte = open(a[a.index('--arte') + 1], encoding='utf-8').read() if '--arte' in a else None
    malla = open(a[a.index('--malla') + 1], 'rb').read() if '--malla' in a else None
    if a[0] == '--poner-icono':
        w = poner_icono(open(a[1], 'rb').read(), arte, malla, a[a.index('--nombre') + 1] if '--nombre' in a else None)
        open(a[2], 'wb').write(w)
        print('%s: icono %s, malla %s | fichero %d B' % (a[2], 'nuevo' if arte else 'el que tenía', '%d B' % len(malla) if malla else 'ninguna', len(w)))
        sys.exit(0)
    rec, api = b'', None
    if '--rec' in a: rec = open(a[a.index('--rec') + 1], 'rb').read()
    if '--api' in a: api = tuple(int(x) for x in a[a.index('--api') + 1].split('.'))
    w, i = empaquetar(open(a[0], 'rb').read(), rec, api, arte, malla)
    open(a[1], 'wb').write(w)
    print('%s: «%s» %s%s perfil %d API %s | fichero %d B | código %d, datos %d, bss %d, %d reubicaciones, recursos %d | pila %d, montón %d | RAM que toma: %d B'
          % (a[1], i['nombre'], i['maquina'], ' Thumb' if i['thumb'] else '', i['perfil'], i['api'], i['fich'], i['cod'], i['dat'], i['bss'], i['reub'], i['rec'], i['pila'], i['monton'], i['ram']))
