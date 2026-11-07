#!/usr/bin/env python3
"""sen_ausentes.py CC SALIDA -- ARGUMENTOS_DE_ENLACE...    (SDK de juegos grandes, api 1.1)

RECORTAR un port: cuando un juego se compila solo con una parte de sus datos (p. ej. solo el nivel que se juega),
el resto del código sigue NOMBRANDO lo que no se ha compilado (tablas de niveles, colisiones de otros niveles…). Este
programa enlaza, recoge cada «undefined reference to `X'» y escribe en SALIDA un `-Wl,--defsym=X=w5_ausente` por
cada uno (un fichero de respuesta para gcc: @SALIDA). La app define w5_ausente() —una función que sale limpia diciendo
que se ha usado algo que el .sen no lleva—. Repite hasta que el enlace sale. Lista lo que ha redirigido, por si acaso:
recortar es una DECISIÓN del port (qué partes del juego no van), no un arreglo."""
import re, subprocess, sys
cc, salida = sys.argv[1], sys.argv[2]
args = sys.argv[sys.argv.index('--') + 1:]
ausentes = set()
for vuelta in range(6):
    with open(salida, 'w') as f:
        f.write(''.join('-Wl,--defsym=%s=w5_ausente\n' % a for a in sorted(ausentes)))
    r = subprocess.run([cc] + args + ['@' + salida], capture_output=True, text=True)
    nuevos = set(re.findall(r"undefined reference to `([^']+)'", r.stderr)) - ausentes
    if r.returncode == 0:
        print('sen_ausentes: %d símbolos redirigidos a w5_ausente (%s)' % (len(ausentes), salida))
        sys.exit(0)
    if not nuevos:
        sys.stderr.write(r.stderr[-4000:]); sys.exit('sen_ausentes: el enlace falla por otra cosa')
    ausentes |= nuevos
sys.exit('sen_ausentes: no converge')
