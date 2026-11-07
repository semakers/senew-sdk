#!/usr/bin/env python3
"""sen_recursos.py UNO.elf UNO.map rec.ld [--informe FICHERO]     (SDK de juegos grandes, api 1.1)

CLASIFICA los datos de solo lectura de un juego entre la IMAGEN (RAM, se reubica al cargar) y la VENTANA DE RECURSOS
(flash, en W5_REC_VA, no se reubica nunca). Es la herramienta MECÁNICA que sustituye a «convertir cada &simbolo en
base de recursos + desplazamiento» a mano: en vez de tocar el código del juego, se decide en el ENLACE dónde vive cada
dato, y como la ventana está siempre en la misma dirección, los punteros a los recursos —desde el código, desde los
datos o desde otros recursos— ya son los buenos.

Entrada: el ENLACE 1 (lib/w5_juego.ld con un rec.ld vacío: todo en la imagen; con -ffunction-sections
-fdata-sections y --emit-relocs) y su mapa. Regla, sección de entrada a sección de entrada:
  - candidatas: las .rodata* (no las fusionables .rodata.str*/.cst*: el enlazador las mezcla y su sitio no es el del mapa);
  - una candidata NO puede ir a la ventana si tiene una reubicación que no sea «palabra absoluta» (R_ARM_ABS32), o una
    palabra absoluta que apunte a algo que NO va a la ventana (código, datos, bss: eso se mueve al cargar y la flash no
    se puede reubicar);
  - punto fijo: se repite hasta que nada cambia. Lo que queda va a la ventana. (Que el código o los datos apunten a la
    ventana es libre: es una dirección fija.)
  - las reubicaciones relativas al contador de programa que CRUZAN de lado (de la imagen a la ventana o al revés)
    también obligan a quedarse en la imagen (la distancia cambiaría al cargar).
Salida: rec.ld, un trozo de guion de enlace con las secciones elegidas en el orden del enlace 1 (lo incluye
lib/w5_juego.ld dentro de .w5rec) y un resumen. El empaquetador vuelve a comprobar, sobre el enlace final, que nada de
la ventana apunta a la imagen."""
import bisect, re, struct, sys

def elf_leer(ruta):
    e = open(ruta, 'rb').read()
    assert e[:4] == b'\x7fELF' and e[4] == 1 and e[5] == 1, 'solo ELF32 little-endian (ARM)'
    shoff, = struct.unpack_from('<I', e, 32); shentsize, shnum, shstrndx = struct.unpack_from('<HHH', e, 46)
    secs = []
    for i in range(shnum):
        n, t, fl, ad, of, sz, lk, inf, al, es = struct.unpack_from('<IIIIIIIIII', e, shoff + i * shentsize)
        secs.append(dict(n=n, t=t, fl=fl, ad=ad, of=of, sz=sz, lk=lk, inf=inf, es=es))
    so = secs[shstrndx]['of']
    for s in secs: s['nombre'] = e[so + s['n']:e.index(b'\0', so + s['n'])].decode()
    return e, secs

def mapa_leer(ruta):
    """secciones de ENTRADA del mapa de ld: (dirección, tamaño, fichero, sección); solo lo que se enlazó."""
    ent, dentro, pend = [], False, None
    for l in open(ruta, errors='replace'):
        if l.startswith('Linker script and memory map'): dentro = True; continue
        if not dentro: continue
        l = l.rstrip('\n')
        m = re.match(r'^ (\.\S+)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S.*)$', l)
        if m:
            pend = None
            ent.append((int(m.group(2), 16), int(m.group(3), 16), m.group(4).strip(), m.group(1))); continue
        m = re.match(r'^ (\.\S+)$', l)
        if m: pend = m.group(1); continue
        m = re.match(r'^\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S.*)$', l)
        if m and pend:
            ent.append((int(m.group(1), 16), int(m.group(2), 16), m.group(3).strip(), pend))
        pend = None
    return [x for x in ent if x[1] > 0]

def patron(fich, sec):
    m = re.match(r'^(.*)\((.*)\)$', fich)                # miembro de una biblioteca: /ruta/libgcc.a(_x.o)
    if m: return '*%s:%s(%s)' % (m.group(1).split('/')[-1], m.group(2), sec)
    return '"%s"(%s)' % (fich, sec)

def main():
    a = sys.argv[1:]
    elf, secs = elf_leer(a[0])
    ent = mapa_leer(a[1])
    texto = [s for s in secs if s['nombre'] == '.text'][0]
    ini_t, fin_t = texto['ad'], texto['ad'] + texto['sz']
    ent = sorted(x for x in ent if ini_t <= x[0] < fin_t or x[3].startswith('.data') or x[3].startswith('.bss'))
    dirs = [x[0] for x in ent]
    def que(v):
        i = bisect.bisect_right(dirs, v) - 1
        if i >= 0 and ent[i][0] <= v < ent[i][0] + ent[i][1]: return i
        if i >= 0 and v == ent[i][0] + ent[i][1]: return i      # uno más allá del final
        return None
    cand = set(i for i, x in enumerate(ent) if x[3].startswith('.rodata') and ini_t <= x[0] < fin_t
               and not re.search(r'\.(str|cst)[0-9]', x[3]))
    malas, aristas, motivo = set(), {}, {}
    sims = {}
    for i, s in enumerate(secs):
        if s['t'] == 2:
            sims[i] = [struct.unpack_from('<IIIBBH', elf, s['of'] + j * 16) for j in range(s['sz'] // 16)]
    for s in secs:
        if s['t'] != 9: continue
        dest = secs[s['inf']]
        if dest['nombre'] not in ('.text', '.data'): continue
        for j in range(s['sz'] // 8):
            off, info = struct.unpack_from('<II', elf, s['of'] + j * 8)
            t = info & 0xFF
            if t == 0: continue
            src = que(off)
            w, = struct.unpack_from('<I', elf, dest['of'] + off - dest['ad'])
            if t in (2, 38):                                    # ABS32 / TARGET1: la palabra es la dirección final
                tgt = que(w)
                if src in cand:
                    aristas.setdefault(src, set()).add(tgt)
            elif t in (3, 42):                                  # REL32 / PREL31: relativas que pueden cruzar
                d = w if t == 3 else (w & 0x7FFFFFFF) | (0x80000000 if w & 0x40000000 else 0)
                tgt = que((off + d) & 0xFFFFFFFF)
                for x in (src, tgt):
                    if x in cand: malas.add(x); motivo[x] = 'relativa que cruza'
            else:
                if src in cand: malas.add(src); motivo[src] = 'reubicación de tipo %d' % t
    eleg = cand - malas
    cambia = True
    while cambia:
        cambia = False
        for i in list(eleg):
            for t in aristas.get(i, ()):
                if t not in eleg:
                    eleg.discard(i); cambia = True
                    motivo[i] = 'apunta a ' + ('código/datos (%s)' % ent[t][3] if t is not None else 'fuera de todo')
                    break
    elegidas = sorted(eleg)
    with open(a[2], 'w') as f:
        f.write('/* GENERADO por cargables/tools/sen_recursos.py desde %s: lo que va a la ventana de recursos. */\n' % a[0])
        for i in elegidas: f.write('    %s\n' % patron(ent[i][2], ent[i][3]))
    tc = sum(ent[i][1] for i in cand); te = sum(ent[i][1] for i in elegidas)
    print('sen_recursos: %d secciones de solo lectura (%d B); a la VENTANA %d (%d B); se quedan en la imagen %d (%d B)'
          % (len(cand), tc, len(elegidas), te, len(cand) - len(elegidas), tc - te))
    if '--informe' in a:
        with open(a[a.index('--informe') + 1], 'w') as f:
            for i in sorted(cand - eleg, key=lambda i: -ent[i][1]):
                f.write('%8d %s %s: %s\n' % (ent[i][1], ent[i][3], ent[i][2], motivo.get(i, '?')))

main()
