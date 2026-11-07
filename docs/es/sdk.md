# Guía del SDK: tu primera app `.sen` de principio a fin

> [!WARNING]
> **senew es un proyecto nuevo; úsalo bajo tu propio riesgo.** El sistema, el SDK y las herramientas son software joven para un teléfono descatalogado. Haz copia de seguridad del teléfono antes de instalar nada y prueba primero en el emulador.

Una app de senew es un fichero **`.sen`**: un binario ARM pequeño que el sistema descubre en la memoria o en la tarjeta, enseña en el menú con su icono, carga, ejecuta y descarga. La app no enlaza con el sistema: lo único que ve es una **tabla de funciones** (la API) que el sistema le entrega al abrirla.
Este SDK trae todo lo necesario para escribir, compilar y empaquetar una app **fuera** del árbol del sistema: cabeceras, guion de enlace, arranque, empaquetador, un `Makefile` reutilizable y una plantilla.

Referencias: [`sdk-api.md`](sdk-api.md) (la API, función a función) · [`apps-con-internet.md`](apps-con-internet.md) (apps que hablan con su propio servidor) · apps de ejemplo reales en [github.com/semakers](https://github.com/semakers) (`senew-app-*`).

## 1. Lo que necesitas

| Herramienta | Para qué | Versiones probadas |
|---|---|---|
| `arm-none-eabi-gcc` (cadena «bare metal») | compilar la app | Arm GNU Toolchain **15.2.Rel1** (macOS arm64) y **15.2.0** (Fedora, paquete `arm-none-eabi-gcc-cs`) |
| GNU make | `Makefile` | 4.x (en macOS, el de Homebrew: `brew install make` y `gmake`/`gnubin` en el `PATH`) |
| Python | empaquetar (`tools/sen_empaquetar.py`) | 3.9 a 3.14 (probadas 3.9.6, 3.12.8 y 3.14.3); **solo biblioteca estándar** |
| git | clonar el SDK | cualquiera |

No hace falta libc, ni newlib, ni un sistema operativo: la app es *freestanding* y solo usa `libgcc` (que trae la cadena) para la aritmética que el ARM926 no hace en hardware (p. ej. la división).

- **macOS**: descarga la *Arm GNU Toolchain* («AArch32 bare-metal target, arm-none-eabi») de <https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>, descomprímela y pon su `bin/` en el `PATH`.
- **Linux**: Debian/Ubuntu `sudo apt install gcc-arm-none-eabi make python3 git`; Fedora `sudo dnf install arm-none-eabi-gcc-cs make python3 git`; o la misma *Arm GNU Toolchain* de arriba.
- **Windows**: lo más cómodo es WSL con las órdenes de Linux. Nativo: *Arm GNU Toolchain* para Windows + `make` y `python` (p. ej. de MSYS2).

Comprueba: `arm-none-eabi-gcc --version` y `python3 --version`.

> [!NOTE]
> Solo se ha probado con las versiones de la tabla. Otras (p. ej. la de Debian) deberían servir, pero si el enlace falla por `libgcc`, usa la *Arm GNU Toolchain* oficial.

## 2. Compila la plantilla (cinco minutos)

```sh
git clone https://github.com/semakers/senew-sdk
cp -r senew-sdk/template mi-app        # la plantilla trae su Makefile
cd mi-app
make SENEW_SDK=../senew-sdk            # -> build/hello.sen
```

Verás algo como:

```
build/hello.sen: «Hello» ARMv5TE perfil 1 API 1.7 | fichero 916 B | código 736, datos 0, bss 16, 13 reubicaciones, recursos 0 | pila 8192, montón 0 | RAM que toma: 9520 B
```

Si pones `mi-app` junto a `senew-sdk`, basta `make`: el `Makefile` busca el SDK en `../senew-sdk` y, si no lo encuentra, lo clona (superficialmente) en `./.senew-sdk`. Con `SENEW_SDK=/ruta` o `make SENEW_SDK=/ruta` eliges otra copia.

**Subirla al teléfono**: abre **SENUS** con el teléfono conectado, entra en **Apps** y usa **«Subir .sen»**. La app sale en el menú (o en «Apps» si el menú está lleno).
**Probarla antes en el emulador**: ver [§8](#8-probar-en-el-emulador).

## 3. Anatomía de una app

La plantilla (`template/src/hello.c`) es un contador; es toda la app:

```c
#include "w5.h"
#include "w5_pantalla.h"     /* (API 1.7) la letra nueva; da W5_PANT_MENOR */

W5_APP(.nombre = "Hello", .version = 1, .api_menor = W5_PANT_MENOR, .caps = W5_CAP_GIRA, .icono_gen = W5_IC_RELOJ);

static int count;            /* el estado de la app: variables estáticas (cada carga empieza en frío) */

static int key(W5Ctx c, int k, void *dato)
{
    (void)dato;
    if (k == W5K_OK) count++;
    else if (k == W5K_C) count = 0;
    else return ui_teclas_defecto(c, k);   /* sel. der. = atrás; el resto, lo de siempre */
    ui_sucio(ui_mi_pantalla(c));           /* el estado cambió: reconstruye la pantalla */
    return 1;                              /* tecla consumida */
}

static W5W build(W5Ctx c, void *dato)
{
    (void)c; (void)dato;
    return Teclas(key, ui_andamio("Hello",
        Columna(A_PRINCIPAL(W5P_CENTRO), A_CRUZADO(W5P_CENTRO), A_ESPACIO(8), HIJOS(
            Texto(ui_fmt("%d", count), A_ESCALA(4), A_COLOR(ui_color(W5_COL_ACENTO))),
            Texto("OK adds one, C resets", A_COLOR(ui_color(W5_COL_TENUE))))),
        "Reset", "Back"));
}

W5Pal app_suceso(int suceso, W5Pal a, W5Pal b)
{
    (void)a; (void)b;
    if (suceso == W5_ABRIR) return (W5Pal)ui_pantalla(build, NULL, NULL, NULL);
    return 0;
}
```

Las piezas:

1. **`W5_APP(...)`** — la identidad de la app: nombre (el del menú, UTF-8, hasta 29 bytes), versión, la **API que necesita** (`.api_menor`), las **capacidades** que exige (`.caps`), el icono genérico del sistema (`.icono_gen`), pila (`.pila`, por omisión 8 KiB) y montón (`.monton`, para `ap_mem_pedir`). Solo lo lee el empaquetador: no ocupa RAM en el teléfono.
2. **`app_suceso()`** — la **única** función que el sistema llama. `W5_ABRIR` devuelve la pantalla raíz; `W5_PAUSA`/`W5_SIGUE` avisan de que la app deja de verse (un aviso del sistema encima, pantalla apagada) y vuelve; `W5_SALIR` es la última ocasión de guardar.
3. **Pantallas** — `ui_pantalla(construir, iniciar, desechar, dato)`. El sistema llama a `construir` cada vez que algo cambió y tú devuelves el **árbol de widgets**. `iniciar` y `desechar` son opcionales (ahí se arrancan y se paran los temporizadores).
4. **Widgets** — `Columna`, `Fila`, `Texto`, `Lista`, `Caja`… con atributos `A_*` (ver [`sdk-api.md`](sdk-api.md) §4). **Un widget dura un cuadro**: nada de guardarlos. El estado va en tus variables.
5. **Teclas** — envuelve la pantalla en `Teclas(manejador, hijo)`; el manejador devuelve 1 si se queda con la tecla. `ui_teclas_defecto` hace lo estándar (selección derecha o «atrás» = salir).
6. **`ui_andamio(título, cuerpo, izq, der)`** — el marco estándar: barra de título, tu cuerpo y las etiquetas de las dos teclas de selección.

### 3.1. El tiempo: nunca esperes en un bucle

La app **no tiene bucle principal propio** (salvo el perfil juego): el sistema la llama por sucesos y por temporizadores. `while (!listo) ;` dentro de la app **no termina nunca** porque nada se mueve mientras tu código no vuelve.

```c
static void cada_segundo(W5Ctx c) { ui_sucio(c); }
static void iniciar(W5Ctx c, void *dato) { (void)dato; ui_cada(c, 1000, cada_segundo); }   /* muere con la pantalla */
```

`ui_cada(ctx, ms, f)` repite, `ui_tras(ctx, ms, f)` una vez, `ui_cancelar` los quita. Los temporizadores mueren solos al cerrarse la pantalla y **no laten en pausa**. Para un cálculo largo, llama a `ap_latido()` dentro del bucle (si no, el perro te cierra a los ~30 s).
Todo lo lento (red, cámara, micrófono) es **asíncrono**: la orden arranca y miras cómo va en un `ui_cada`.

### 3.2. Memoria

- **Pila**: 4 KiB a 256 KiB (`.pila`); por omisión 8 KiB. No pongas arreglos grandes en la pila.
- **Estáticos** (`.data`, `.bss`): cuentan en la RAM de la app. Para ver cuánto toma, mira la última columna que imprime el empaquetador («RAM que toma»).
- **Montón**: pídelo en `W5_APP(.monton = …)` y usa `ap_mem_pedir(n)` / `ap_mem_soltar` / `ap_mem_libre` (hasta 14 MiB; viene a cero; `NULL` si no cabe). No hay `malloc`.
- **Arena por cuadro**: `ui_fmt()` y `ui_arena(n)` dan memoria que **dura un cuadro**. Si quieres conservar un texto, cópialo a un búfer tuyo.
- **Recursos** grandes de solo lectura (niveles, sonidos, texturas) van **al final del `.sen`** (`--rec fichero`, que `mk/app.mk` pasa solo si existe `resources.bin`) y no ocupan RAM: se leen a trozos con `ap_rec_leer`, o en el sitio con `ap_rec_mapa` (puede devolver `NULL`).
- Sin coma flotante en hardware: usa enteros o punto fijo (la `senew-app-calculadora` es de punto fijo de 4 decimales). El compilador puede emitir llamadas a `libgcc` para flotantes, pero es lento.

### 3.3. Ficheros

- `ap_fich_leer/escribir/borrar/tam(nombre, …)`: ficheros **propios** de la app. El sistema les pone un prefijo con el nombre de la app, así que no ves ni pisas los de otras. Escribir es atómico (se guarda entero) y pide `W5_CAP_FICHEROS`. Hay un tope de 20 escrituras por minuto (desgaste de la flash).
- `fs_tam/fs_leer/fs_listar`: **solo lectura** de cualquier fichero o carpeta del teléfono (memoria y tarjeta).
- `ma_*`: «Mis archivos», la carpeta del usuario (por ejemplo, guardar una nota como `.txt`).

### 3.4. Capacidades y versión de la API

Cada aparato dice qué tiene (`ap_consulta(W5_Q_CAPS)`: acelerómetro, vibrador, luces, joystick, teclado numérico…) y qué tamaño tiene su pantalla (`W5_Q_ANCHO/ALTO`). Si pones una capacidad en `.caps`, la app **solo carga** donde exista; si es opcional, no la pidas y pregunta en tiempo de ejecución. Los widgets se recolocan solos; **declara `W5_CAP_GIRA`** si tu interfaz soporta girar con el acelerómetro.

`.api_menor` es la versión menor de la API con que compilas: el sistema rechaza (con un mensaje claro) una app que pide más de lo que tiene. Pide **lo mínimo**: cada cabecera `w5_*.h` documenta su menor (`W5_PANT_MENOR` = 7, `W5_CAM_MENOR` = 9…). Los grupos opcionales (red, música por la red, cámara, micrófono, sonido, descargas) se miran antes de usarlos: `w5_red_hay()`, `w5_mured_hay()`…

### 3.5. Iconos

- `.icono_gen = W5_IC_*`: un icono genérico del sistema (calculadora, reloj, notas, juego…).
- `icon/arte.txt` (opcional): 12 líneas del mismo largo (1 a 16 caracteres), `#` = tinta; es el icono plano de 1 bit que va en la cabecera del `.sen`. `mk/app.mk` lo usa solo si existe.
- `icon/malla.bin` (opcional): una malla 3D (hasta 256 triángulos) para los estilos 3D del menú. Su formato está en `include/cargable.h` (`W5xIcoMalla`); el empaquetador la valida igual que el teléfono.

## 4. Estructura de un proyecto

```
mi-app/
  Makefile        APP = mi_app + la parte que busca el SDK + include $(SENEW_SDK)/mk/app.mk
  src/*.c, *.h    todo se compila junto (src/ está en el -I)
  icon/arte.txt   opcional
  icon/malla.bin  opcional
  resources.bin   opcional (recursos de solo lectura)
```

Variables útiles del `Makefile` (antes del `include`): `APP`, `SRCS`, `EXTRA_SRCS`, `EXTRA_CFLAGS` (p. ej. `-O2`), `ICON_ART`, `ICON_MESH`, `RES`, `ARMCC`, `PYTHON`, `BUILD`. Están todas comentadas en [`mk/app.mk`](../../mk/app.mk). Destinos: `make`, `make size`, `make clean`.

Las banderas de compilación son fijas y conviene no tocarlas: `-mcpu=arm926ej-s -mfloat-abi=soft -marm -mthumb-interwork -ffreestanding -fno-builtin -fno-pic -Os …`. Si quieres Thumb para ahorrar código, compila tus fuentes con `-mthumb` (`EXTRA_CFLAGS = -mthumb`): el sistema llama a la app con interworking. Lo que **no** puedes usar es PIC (`-fpic`): la app se enlaza en la dirección 0 y el sistema la reubica sumando la base a una lista de palabras.

## 5. El empaquetador: del ELF al `.sen`

`tools/sen_empaquetar.py app.elf app.sen [--rec recursos.bin] [--arte icono.txt] [--malla malla.bin] [--api MAYOR.MENOR]` lee el ELF (enlazado en 0 con `lib/w5_app.ld` y `--emit-relocs`), comprueba que **todas** las reubicaciones sean «palabra absoluta» o «relativa al contador de programa», construye la imagen (código, datos, bss), la lista de reubicaciones, los recursos y la cabecera de 128 bytes con sus CRC, y escribe el `.sen`.
Lo difícil se decide aquí, en tu ordenador: si hay algo que el teléfono no sabe reubicar, **falla el empaquetado** con un mensaje (por ejemplo «compila sin -fpic y para ARMv5»), no el teléfono. También cambia el icono de un `.sen` ya hecho: `sen_empaquetar.py --poner-icono entra.sen sale.sen --arte icono.txt`.

El formato exacto (cabecera `W5xCab`, límites, CRC) está en `include/cargable.h` §1.

## 6. Errores típicos

| Mensaje | Causa y arreglo |
|---|---|
| `falta W5_APP(...) en la app (sección .w5meta)` | Olvidaste la línea `W5_APP(...)` (o no se compiló ese fichero). |
| `reubicación de tipo N … no se puede llevar a un .sen` | Compilaste con PIC o para otra arquitectura; usa las banderas de `mk/app.mk`. |
| `la palabra en 0x… apunta fuera de la app` | Un puntero a una dirección fija (p. ej. un registro de hardware): una app no toca el hardware, usa la API. |
| `undefined reference to 'memcpy'` etc. | `w5_crt0.c` las define: asegúrate de que `mk/app.mk` lo enlaza (no uses otro Makefile sin copiarlo). |
| La app sale del menú pero «se cierra» al abrir con un mensaje | Mira el mensaje: falta de capacidad, API más nueva que la del sistema, pila desbordada, puntero fuera de la app… |
| `Pide la API 1.N y este sistema tiene la 1.M` | Baja `.api_menor` o actualiza senew desde SENUS. |
| El texto de un `ui_fmt` «se corrompe» al día siguiente | Dura un cuadro: cópialo. |

## 7. Ejemplos de verdad

Cada repo compila con este SDK y es una app que viene (o vino) de fábrica o se publica en la tienda de SENUS:

| Quieres hacer… | Mira |
|---|---|
| Lo más básico: estado, teclas, temporizador, hora | `senew-app-ejemplo` («Cuentavueltas»), `template/` |
| Una lista, diálogos, entrada de texto, ficheros propios | `senew-app-notas` |
| Un juego con bucle propio de widgets | `senew-app-serpiente` |
| Componentes del sistema de diseño | `examples/componentes` (en este repo), `senew-app-calculadora`, `senew-app-luces` |
| Hablar con internet o con tu servidor | `senew-app-plantilla_red`, `senew-app-red`, y [`apps-con-internet.md`](apps-con-internet.md) |
| Música por la red / vídeo | `senew-app-musica`, `senew-app-videos` |
| La cámara / el micrófono / el sonido y el vibrador | `senew-app-camara_prueba`, `senew-app-grabadora`, `senew-app-sonido` |
| Acelerómetro, pasos, teclado crudo, puerto serie | `senew-app-movimiento`, `senew-app-teclas`, `senew-app-consola` |
| Imágenes (GIF/PNG/JPEG) | `senew-app-imagenes` |
| Un juego grande (perfil juego: pantalla completa y bucle propio, montón grande) | `senew-doom` |

## 8. Probar en el emulador

senew corre en un modelo QEMU del W580i: [`senew-emu`](https://github.com/semakers/senew-emu). El sistema (`senew-w580`) genera con `make w580` una imagen de flash para el emulador (`build/flash-w580.bin`, **solo para el emulador**: no se graba en un teléfono). Los pasos exactos para precargar un `.sen` en la memoria emulada y manejarlo con el teclado están en el README de [`senew-w580`](https://github.com/semakers/senew-w580), sección «Probar una app en el emulador».
También puedes probar el sistema en el navegador en <https://semakers.com/probar/> (el emulador oficial; no admite subir tus propias apps).

## 9. Probar en el teléfono

1. Instala senew con SENUS (ver <https://semakers.com>), **con copia de seguridad hecha**.
2. SENUS → **Apps** → **Subir .sen**. Para quitarla, el mismo apartado.
3. Una app que falla **no tumba el teléfono**: el sistema la cierra y dice por qué («App cerrada: motivo»). Lo que sí puede pasar es que quede lenta o gaste batería: prueba despacio.

## 10. Versiones del SDK y compatibilidad

La API del sistema es `1.N` y **solo crece por el final**: una app compilada hoy seguirá funcionando en sistemas futuros de la misma mayor. `api/cargable_v1.lst` es la lista congelada de las primeras funciones; `tools/hacer_api.py cargable.h --comprobar api/cargable_v1.lst` comprueba que ninguna cambió de sitio. Para regenerar `include/w5_nombres.h` (los nombres cortos) tras tocar `cargable.h`: `python3 tools/hacer_api.py include/cargable.h --nombres include/w5_nombres.h`.

## 11. Licencia y contribuciones

El SDK y las apps son **MIT**. El sistema (`senew-w580`) y SENUS son GPL-3.0; una app `.sen` no enlaza con ellos (solo habla por la tabla), así que **tu app puede llevar la licencia que quieras**.
Issues y pull requests, bienvenidos en <https://github.com/semakers/senew-sdk>. Los comentarios de las cabeceras están en español, la lengua de trabajo del proyecto; esta guía y [`sdk-api.md`](sdk-api.md) existen también en inglés (`docs/en/`).
