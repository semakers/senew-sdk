#!/usr/bin/env python3
"""hacer_api.py — de la lista W5_API de cargable.h saca:
   sdk/w5_nombres.h (los nombres cortos), una lista numerada (para congelarla) y la tabla en Markdown.
   uso: hacer_api.py cargable.h --nombres w5_nombres.h --lista api.lst --md api.md
        hacer_api.py cargable.h --comprobar api_congelada.lst   (la tabla solo puede CRECER POR EL FINAL)"""
import re, sys
def grupos(t):
    """(1.2) los grupos de la tabla: `#define W5_API12_X(F) \\` seguido de sus F(...), una por línea"""
    g = {}
    for m in re.finditer(r'#define (W5_API1[23]\w*)\(F\)((?:[^\n]*\\\n)*[^\n]*)\n', t):
        g[m.group(1)] = m.group(2)
    return g
def leer(ruta):
    t = open(ruta, encoding='utf-8').read()
    cuerpo = t[t.index('#define W5_API(F)'):t.index('#define W5_API_MAGIA')]
    gs = grupos(t)                                   # (1.2) un grupo citado en el cuerpo se lee en su sitio, en su orden
    cuerpo = re.sub(r'(W5_API1[23]\w*)\(F\)', lambda m: '\n' + gs.get(m.group(1), '') + '\n', cuerpo)
    fs, grupo = [], ''
    for lin in cuerpo.split('\n'):
        g = re.match(r'\s*/\* \d+: (.*?) \*/', lin)
        if g: grupo = g.group(1); continue
        m = re.match(r'\s*F\((.+?), (\w+), \((.*?)\)\)\s*(?:/\*\s*(.*?)\s*\*/)?', lin)
        if m: fs.append((m.group(2), m.group(1), m.group(3), m.group(4) or '', grupo))
    return fs
def main():
    a = sys.argv[1:]; fs = leer(a[0])
    op = dict(zip(a[1::2], a[2::2]))
    lista = ''.join('%d %s %s(%s)\n' % (i, f[0], f[1], f[2]) for i, f in enumerate(fs))
    if '--nombres' in op:
        s = '/* w5_nombres.h — GENERADO por tools/hacer_api.py desde cargable.h: no tocar. */\n'
        s += ''.join('#define %s (w5->%s)\n' % (f[0], f[0]) for f in fs if not f[0].startswith(('mem', 'str')))
        s += '#define strlen (w5->strlen)\n#define strcmp (w5->strcmp)\n'
        open(op['--nombres'], 'w').write(s)
    if '--lista' in op: open(op['--lista'], 'w').write(lista)
    if '--md' in op:
        s = '| n.º | función | qué hace |\n|---|---|---|\n'; g = None
        for i, f in enumerate(fs):
            if f[4] != g: g = f[4]; s += '| | **%s** | |\n' % g
            s += '| %d | `%s %s(%s)` | %s |\n' % (i, f[1], f[0], f[2].replace('|', '\\|'), f[3])
        open(op['--md'], 'w').write(s)
    if '--comprobar' in op:
        viejo = open(op['--comprobar']).read()
        if not lista.startswith(viejo):
            print('LA TABLA HA CAMBIADO por en medio: una API publicada solo crece por el final'); sys.exit(1)
        print('tabla: %d funciones; las %d congeladas siguen en su sitio' % (len(fs), viejo.count('\n')))
    print('%d funciones' % len(fs))
main()
