# `.sen` app API reference

> [!WARNING]
> **senew is a new project; use it at your own risk.** The API is stable by construction (it only grows at the end), but the system is young and there will be bugs.

This reference is derived from the headers in [`include/`](../../include) (`cargable.h` and `w5_*.h`), which are the source of truth: if something does not match, the header wins.
For an end-to-end guide read [`sdk.md`](sdk.md) first; for apps that talk to the internet, [`apps-con-internet.md`](apps-con-internet.md).
(The comments inside the headers are in Spanish, the project's working language; this document is the English reading guide.)

## 1. How the API works

- An app **does not link against the system**. When it is opened the system hands it a pointer to a **function table** (`const W5Api *w5`) and the app calls everything through it.
  The headers define short names so you do not write `w5->`: `ui_sucio(c)` is `w5->ui_sucio(c)`.
- The table has a **MAJOR.MINOR** version. Within a major, **the table only grows at the end**, and no function, widget attribute, query or key ever changes number or meaning.
  An app declares the version it was built against (`W5_APP(.api_menor = …)`): it loads if `major == the system's` and `minor <= the system's`; if it asks for more, the system says "a newer system is needed".
- The base is in `cargable.h` (1.0 to 1.3, 150 functions). Later groups live in separate headers that you include **after `w5.h`** and that are **optional** (an app using them must either load on a system without them, or declare the API it needs):
  `w5_sonido.h` (1.5), `w5_red.h` (1.6), `w5_pantalla.h` (1.7), `w5_musica_red.h` (1.8), `w5_camara.h` (1.9), `w5_microfono.h` (1.10), `w5_descargas.h` (1.11).
  Before calling an optional group, ask whether it exists: `w5_red_hay()`, `w5_mured_hay()`, `w5_son_hay()`, `w5_pixeles_hay()`… (each header has its own).
- **Nothing about the phone model** is in the API: the screen size (`W5_Q_ANCHO/W5_Q_ALTO`), which keys exist (`W5_Q_TECLAS_A/B`) and the capabilities (`W5_Q_CAPS`) are **queried** with `ap_consulta()`. The same binary runs on a 240x320 screen and on a 176x220 one.
- **No structure crosses the table by value.** A few cross by pointer, fixed field by field in `cargable.h` (`W5Rect`, `W5Fecha`, `W5Icono`, `W5Nota`…). Widgets, the context, the canvas and the text entry are **opaque handles**.
- **ABI** (machine 1, the phone): ARMv5TE, little-endian, AAPCS, **soft-float** (floats never cross the table), ARM or Thumb with interworking. Types: `char` 8, `short` 16, `int` 32, pointer = 32 bits. Colors are `0x00RRGGBB` (`W5_TRANSPARENTE` = "the theme's").

## 2. What the system asks of the app

One function: `W5Pal app_suceso(int suceso, W5Pal a, W5Pal b)`. Events: `W5_ABRIR` (return the root screen made with `ui_pantalla(...)`, or 0 if you cannot start), `W5_PAUSA` / `W5_SIGUE` (the app stops being visible / is visible again; no ticks in between) and `W5_SALIR` (last chance to save).
With the `W5_PERFIL_JUEGO` profile, `W5_ABRIR` does not return until the game ends (the app keeps the screen and the loop: `jg_*`).

## 3. Capabilities, queries, keys

- **Capabilities** (`W5_CAP_*`): what the app **needs**; if the device lacks one, the app is not loaded. Optional ones are not requested: ask with `ap_consulta(W5_Q_CAPS)`.
  `ACELEROMETRO` (accelerometer), `VIBRADOR`, `ZUMBADOR` (buzzer), `LUCES`, `TAPA` (lid/slider), `FICHEROS`, `RELOJ`, `COLOR`, `TECLADO_NUM`, `JOYSTICK`, `REC_VENTANA`, `JIT`, `TECLAS_CRUDAS`, `GIRA` (the app knows how to rotate with the screen), `MUSICA`, `RED`, `IMAGENES`, `SERIE`, `PASOS`, `LUCES_EF`… (see the headers for the full list).
- **Queries** (`ap_consulta`): `W5_Q_API, ANCHO, ALTO, CAPS, TECLAS_A, TECLAS_B, ALTO_LINEA, MODELO, PRIMER_PLANO, PILA_LIBRE, ACEL_1G, PIXEL, AVISOS`. A query that does not exist returns -1.
- **Keys** (`W5K_*`, abstract codes identical on every model): `ARRIBA, ABAJO, IZQ, DER` (up, down, left, right), `OK, SEL_IZQ, SEL_DER` (soft keys), `ATRAS` (back), `C`, `0…9`, `AST` (*), `ALM` (#), `VOL_MAS, VOL_MENOS, WEB, PLAY, JUEGO_A, JUEGO_B, WALKMAN, CAMARA, BLOQUEO, ENCENDIDO`.
- **Return codes**: `W5_OK = 0`, `W5_NO_DISPONIBLE = -1` (not available on this device / right now), `W5_ERROR = -2` (bad argument). Absurd arguments **close the app** with a message; they never hang the phone.

## 4. Widgets

You describe the UI by returning a widget tree from a `construir` function. In the SDK they are written with macros: `Columna(A_CRUZADO(W5P_ESTIRAR), HIJOS(a, b))`, `Texto("hola", A_COLOR(c), A_ESCALA(2))`.
Underneath it is `ui_w(class, pairs, n)` with {attribute, value} pairs: whatever you leave out takes its default; new classes and attributes never break old apps.

Classes: `Columna`/`Fila` (column/row flex), `Expandido` (expanded), `Hueco` (spacer), `Relleno` (padding), `Centro`/`Alinear` (center/align), `Caja` (box), `Pila` (stack), `Rejilla` (grid), `Lista` (list), `ListaPerezosa` (lazy list), `Texto` (text), `Parrafo` (paragraph), `Velo` (scrim), `BarraDesp` (scroll bar), `Progreso` (progress), `Pintura` (you paint on a canvas with `li_*`), `Selector`, `CampoTexto` (text field), `Icon`, `Enfocable` (focusable), `Teclas` (key handler).
Attributes: `A_CLAVE, A_HIJO, A_AN, A_AL, A_COLOR, A_BORDE, A_FONDO, A_SOMBRA, A_BARRA, A_PRINCIPAL, A_CRUZADO, A_ENCOGE, A_ESPACIO, A_FLEX, A_COLUMNAS, A_ALTO_CELDA, A_ESCALA, A_MARCAS, A_MAX_LINEAS, A_CENTRADO, A_VALOR, A_MAX, A_MIN, A_DATO, A_VERSION, A_N, A_AL_ACTIVAR, A_AL_MOVER, A_SEL, A_ALTO_FILA, A_AUTOFOCO, A_AL_CAMBIAR, A_DIGITOS, A_LINEAS, A_AL_ENFOCAR, A_ANIM_MS`.
Exact definitions in `include/w5.h` and `include/cargable.h` (section 6).

A widget **lives for one frame**: it is built in `construir()` and returned; state lives in the app's static variables. To refresh call `ui_sucio(ui_mi_pantalla(c))`. Strings from `ui_fmt` also last one frame: **copy them** if you want to keep them.
Values are validated by type: a string must be in the app's memory, in the arena or in the system; a pointer-to-int must be inside the app's memory; a callback must be inside its code.

## 5. Limits

| What | Limit |
|---|---|
| App stack | 4 KiB to 256 KiB (default 8 KiB: `.pila` in `W5_APP`) |
| Heap (`ap_mem_pedir`, `.monton`) | up to 14 MiB |
| Image (code + data + bss) | up to 8 MiB (API 1.1; 4 MiB in 1.0) |
| Relocations | up to 65,536 |
| Read-only resources (`--rec`) | up to 16 MiB; read with `ap_rec_leer` / `ap_rec_mapa`, they take no RAM |
| Own files (`ap_fich_*`) | carry the app's prefix; at most 20 writes per minute (flash wear) |
| Long computations | the watchdog allows about 30 s per call into the app; `ap_latido()` renews it (up to 60 s per call) |
| Apps at a time | one |

If the app fails (CPU exception, hang, stack overflow, writes next to its memory, absurd pointers to the API) the system closes it, says why and carries on. Some guarantees cannot be enforced by the hardware, so they are partial: **an app is not a perfect sandbox**.

## 6. Function tables

The numbers are the slots of the table in `cargable.h` (groups from 1.5 on hang after 1.3 plus the Bluetooth group, so they carry no number here). The `W5_*` codes are defined in the headers.

### Base (standard C library)

| # | function | what it does |
|---|---|---|
| 0 | `void * memcpy(void *d, const void *s, size_t n)` | Standard C library function. |
| 1 | `void * memset(void *d, int c, size_t n)` | Standard C library function. |
| 2 | `void * memmove(void *d, const void *s, size_t n)` | Standard C library function. |
| 3 | `int memcmp(const void *a, const void *b, size_t n)` | Standard C library function. |
| 4 | `size_t strlen(const char *s)` | Standard C library function. |
| 5 | `int strcmp(const char *a, const char *b)` | Standard C library function. |

### The app and the device

| # | function | what it does |
|---|---|---|
| 6 | `int ap_consulta(int que)` | Query a device property by `W5_Q_*` code; returns -1 if that query does not exist. |
| 7 | `void ap_salir(void)` | Close the app (like pressing Back on its root screen). |
| 8 | `void ap_latido(void)` | Tell the system the app is still alive during a long computation; each call buys at most 60 s. |
| 9 | `void * ap_mem_pedir(unsigned n)` | Allocate `n` zero-filled bytes from the app's own heap; returns NULL if it does not fit. |
| 10 | `void ap_mem_soltar(void *p)` | Free a block obtained from `ap_mem_pedir`. |
| 11 | `unsigned ap_mem_libre(void)` | Return the number of bytes still free in the app's heap. |
| 12 | `void ap_log(const char *s)` | Emit a debug trace message. |
| 13 | `unsigned ap_ms(void)` | Return the milliseconds elapsed since boot. |
| 14 | `int ap_fecha(W5Fecha *f)` | Fill a `W5Fecha` with the local time; returns `W5_ERROR` if the clock is not set. |
| 15 | `char * ap_hora_texto(int con_segundos)` | Return the current time as text in the user's format, with or without seconds; valid for one frame. |

### Files (an app's own files carry its prefix, added by the system; it cannot see anyone else's)

| # | function | what it does |
|---|---|---|
| 16 | `int ap_fich_tam(const char *nombre)` | Return the size in bytes of one of the app's own files, or `W5_NO_DISPONIBLE`. |
| 17 | `int ap_fich_leer(const char *nombre, unsigned desde, void *buf, unsigned max)` | Read up to `max` bytes of an app file into `buf`, starting at offset `desde`. |
| 18 | `int ap_fich_escribir(const char *nombre, const void *d, unsigned n)` | Write an app file, whole and atomically. |
| 19 | `int ap_fich_borrar(const char *nombre)` | Delete one of the app's own files. |
| 20 | `int fs_tam(const char *ruta)` | Return the size in bytes of any file on the phone (read-only). |
| 21 | `int fs_leer(const char *ruta, unsigned desde, void *buf, unsigned max)` | Read up to `max` bytes of any phone file into `buf`, starting at offset `desde` (read-only). |

### Keys, sensors and sound (all optional: check `W5_Q_CAPS`)

| # | function | what it does |
|---|---|---|
| 22 | `int ap_tecla_abajo(int k)` | Return 1 if key `k` is pressed right now. |
| 23 | `int ap_acel(int encendido)` | Request (non-zero) or release (0) the accelerometer. |
| 24 | `int ap_acel_leer(int *x, int *y)` | Read the accelerometer into `x` and `y` as signed counts; 0 = flat. |
| 25 | `int ap_vibrar(int ms)` | Vibrate for `ms` milliseconds; returns `W5_NO_DISPONIBLE` without a vibrator. |
| 26 | `int ap_tono(unsigned hz, unsigned ms)` | Play a tone of `hz` Hz for `ms` milliseconds; it stops by itself. |
| 27 | `int ap_sonar(int cual)` | Play system sound `cual` (`W5_SN_*`) at the user's volume. |
| 28 | `int ap_bateria(void)` | Return the battery level 0..100, or `W5_NO_DISPONIBLE`. |

### Building the interface

| # | function | what it does |
|---|---|---|
| 29 | `W5W ui_w(unsigned clase, const W5Pal *pares, unsigned n)` | Build a widget of class `clase` from `n` {attribute, value} word pairs; omitted attributes take their defaults. |
| 30 | `W5W * ui_lista(const W5W *v, unsigned n)` | Build a widget list from `v`: copies it, drops null entries and ends with a null. |
| 31 | `char * ui_fmt(const char *f, ...)` | printf-style formatter supporting `%d %u %x %s %c` with zero padding and width; the result is valid for one frame. |
| 32 | `void * ui_arena(unsigned n)` | Allocate `n` bytes from the per-frame arena; the memory is valid for one frame. |
| 33 | `W5W ui_pantalla(W5Construir construir, W5Vida iniciar, W5Vida desechar, void *dato)` | Create a screen from a build callback, init and dispose callbacks and a user data pointer. |
| 34 | `W5W ui_andamio(const char *titulo, W5W cuerpo, const char *izq, const char *der)` | Build the standard frame: title bar, body and left/right softkey labels. |
| 35 | `W5W ui_fila(const char *texto, const char *detalle, int sel)` | Build a simple list row from `texto` and `detalle`; `sel` marks it as selected. |
| 36 | `W5W ui_dato(const char *nombre, const char *valor)` | Build a name/value data row. |
| 37 | `unsigned ui_color(int rol)` | Return the color for role `rol` (`W5_COL_*`) from the active theme palette. |
| 38 | `void ui_sucio(W5Ctx c)` | Mark context `c` as changed so that its screen is rebuilt. |
| 39 | `W5Ctx ui_mi_pantalla(W5Ctx c)` | Return the app screen context that contains `c`. |
| 40 | `int ui_cada(W5Ctx c, unsigned ms, W5Retro f)` | Run callback `f` every `ms` ms on context `c`; timers die with `c` and do not tick while paused. |
| 41 | `int ui_tras(W5Ctx c, unsigned ms, W5Retro f)` | Run callback `f` once, `ms` milliseconds from now, on context `c`. |
| 42 | `void ui_cancelar(W5Ctx c, W5Retro f)` | Cancel the timer callback `f` registered on context `c`. |
| 43 | `void ui_empujar(W5W pantalla)` | Push another app screen on top of the current one. |
| 44 | `void ui_sacar(void)` | Pop the top app screen, returning to the one below. |
| 45 | `void ui_tecla(int k)` | Deliver key `k` to the UI as if it had been pressed. |
| 46 | `int ui_teclas_defecto(W5Ctx c, int k)` | Default key handling: left softkey = OK, right softkey / Back = exit. |
| 47 | `void ui_aviso(const char *titulo, const char *texto)` | Show a system alert dialog with a title and text. |
| 48 | `void ui_confirmar(const char *titulo, const char *texto, W5Si si, void *dato)` | Show a confirmation dialog with a title and text; calls `si(dato)` if the user accepts. |
| 49 | `void ui_opciones(const char *titulo, const char *const *ops, int n, W5Elegir elegir, void *dato)` | Show a dialog with `n` options; calls `elegir(i, dato)` with the chosen index. |

### Painting (canvas drawing)

| # | function | what it does |
|---|---|---|
| 50 | `void li_rect(W5Lienzo l, int x, int y, int w, int h, unsigned color)` | Fill a rectangle on the canvas with `color`. |
| 51 | `void li_marco(W5Lienzo l, int x, int y, int w, int h, unsigned color)` | Draw a rectangle outline on the canvas in `color`. |
| 52 | `void li_linea_h(W5Lienzo l, int x, int y, int w, unsigned color)` | Draw a horizontal line of width `w` on the canvas. |
| 53 | `void li_punto(W5Lienzo l, int x, int y, unsigned color)` | Draw a single pixel on the canvas. |
| 54 | `void li_texto(W5Lienzo l, int x, int y, const char *utf8, unsigned color, int escala, int marcas)` | Draw UTF-8 text at (`x`, `y`) with `color`, text scale and flags (`marcas`). |
| 55 | `void li_texto_n(W5Lienzo l, int x, int y, const char *utf8, int bytes, unsigned color, int escala, int marcas)` | Draw at most `bytes` bytes of UTF-8 text at (`x`, `y`) with `color`, scale and flags. |
| 56 | `int li_ancho_texto(const char *utf8, int escala, int marcas)` | Return the width in pixels of a UTF-8 text at the given scale and flags. |
| 57 | `int li_partir(const char *utf8, int ancho, int escala, int marcas)` | Return how many bytes of the text fit in `ancho` pixels, breaking at the last space or newline (word wrap). |
| 58 | `void li_velo(W5Lienzo l, int x, int y, int w, int h)` | Dim to half brightness whatever is already painted in the given rectangle (a veil). |
| 59 | `void li_icono(W5Lienzo l, int x, int y, const W5Icono *i, unsigned color, int escala)` | Draw a 1-bit `W5Icono` at (`x`, `y`) in `color` and scale. |

### Text input

| # | function | what it does |
|---|---|---|
| 60 | `W5Et et_nueva(unsigned cap)` | Create a text-input handle with capacity `cap`, from the app's heap; returns NULL if it does not fit. |
| 61 | `void et_poner(W5Et e, const char *s)` | Replace the contents of text input `e` with string `s`. |
| 62 | `const char * et_texto(W5Et e)` | Return the current text of input `e`. |
| 63 | `unsigned et_len(W5Et e)` | Return the length in bytes of the text in input `e`. |
| 64 | `int et_insertar(W5Et e, const char *utf8)` | Insert UTF-8 text at the cursor of input `e`; returns 0 if it does not fit. |
| 65 | `int et_borrar(W5Et e)` | Delete the character before the cursor of input `e` (backspace); returns 0 if there is none. |
| 66 | `const char * et_modo(W5Et e)` | Return the current input mode as text: "abc", "Abc", "ABC" or "123". |

### Large read-only resources (stored at the end of the `.sen`; they take no RAM)

| # | function | what it does |
|---|---|---|
| 67 | `unsigned ap_rec_tam(void)` | Return the size in bytes of the app's read-only resources. |
| 68 | `int ap_rec_leer(unsigned desde, void *buf, unsigned max)` | Read up to `max` bytes of the resources into `buf`, starting at offset `desde`; returns the bytes read. |
| 69 | `const void * ap_rec_mapa(unsigned desde, unsigned n)` | Return an in-place pointer to the resources if the file is contiguous in mapped flash; otherwise NULL (use `ap_rec_leer`). |
| 70 | `int ap_rec_comprobar(void)` | Recompute the CRC of the resources (slow): `W5_OK` or `W5_ERROR`. |

### Game profile: full screen and own loop (only apps with the `W5_PERFIL_JUEGO` profile)

| # | function | what it does |
|---|---|---|
| 71 | `void * jg_tomar(int modo, int *an, int *al, int *paso)` | Take over the screen: returns the frame pointer (format `W5_Q_PIXEL`) or NULL, and reports width, height and stride. |
| 72 | `void jg_volcar(void)` | Push the frame to the panel; also acts as a heartbeat. |
| 73 | `void jg_soltar(void)` | Give the screen back; afterwards the only thing left is to return from `W5_ABRIR`. |
| 74 | `void jg_espera(void)` | Sleep until the next interrupt (at most one tick). |
| 75 | `int jg_teclas(unsigned *a, unsigned *b)` | Read ALL keys pressed now as bitmasks: bit k of `a` (k < 32) or of `b`; returns how many. |
| 76 | `int jg_melodia(const W5Nota *notas, unsigned n, int repetir_desde)` | Play `n` notes through the buzzer, handled by the system; NULL silences it. |

### App services (1.2): settings, time, timer, holds, serial port, raw keys, pedometer, wallpaper, JIT

| # | function | what it does |
|---|---|---|
| 77 | `int ap_jit_sincronizar(const void *ini, unsigned n)` | With `W5_CAP_JIT`: declare that freshly written code in [`ini`, `ini`+`n`) of the app's memory is now executable. |
| 79 | `int ap_ajuste(int clave)` | Return the value of a `W5_AJ_*` setting; `W5_ERROR` if the key is not in the allowed list. |
| 80 | `int ap_ajuste_poner(int clave, int valor, int guardar)` | Change a setting (range-validated; not `W5_AJ_HORA12`); `guardar` = 1 writes it to flash immediately. |
| 81 | `int ap_hora_poner(const W5Fecha *f)` | Set the clock to LOCAL time (year, month, day, hour, min, sec); `W5_ERROR` if the date does not exist. |
| 82 | `char * ap_zona_texto(void)` | Return the user's time zone as text, e.g. "UTC-4:00"; valid for one frame. |
| 83 | `int ap_temporizador_poner(unsigned ms)` | Set the SYSTEM countdown to expire in `ms` ms (it alerts even if the app is closed); 0 stops it. |
| 84 | `int ap_temporizador_queda(void)` | Return the milliseconds left on the countdown, or `W5_NO_DISPONIBLE` if none is running. |
| 85 | `int ap_retener(int que, int si)` | Request (1) or release (0) a `W5_RET_*` hold; all holds are released when the app unloads. |
| 86 | `int ap_pasos(W5Pasos *p)` | COPIES a snapshot of the pedometer into `p`; `W5_NO_DISPONIBLE` without `W5_CAP_PASOS`. |
| 87 | `int ap_pasos_a_cero(void)` | Reset the system step counter (the standby one) to zero. |
| 88 | `int ap_serie_leer(void *buf, unsigned max)` | Non-blocking read from the serial port: returns bytes received (0 if none); `W5_NO_DISPONIBLE` without `W5_CAP_SERIE`. |
| 89 | `int ap_serie_escribir(const void *d, unsigned n)` | Write to the serial port; returns bytes accepted (0 with no cable; it does not hang). |
| 90 | `int ap_serie_conectado(void)` | Return 1 if something is connected at the other end of the cable. |
| 91 | `int ap_teclas_mapa(unsigned char *col)` | COPIES 7 bytes of the raw key matrix: `col[f]` bit k = cell f*5+k pressed (f = 6: grounded keys); returns how many. |
| 92 | `int ap_celda_nombre(int celda, char *buf, unsigned max)` | COPIES the real name of matrix cell 0..34 (e.g. "Vol+") into `buf`, empty if no key; returns its length. |
| 135 | `int ap_poner_fondo(const char *ruta)` | Set the standby wallpaper ("" removes it): `W5_OK`; `W5_ERROR` (could not load); `W5_NO_DISPONIBLE` (did not fit). |
| 136 | `int ap_argumento(char *buf, unsigned max)` | COPIES what the system opened the app with (empty from the menu), 0-terminated, into `buf`; returns its length or `W5_ERROR`. |
| 137 | `int ap_luz(int n, int encendida)` | With `W5_CAP_LUCES`: turn light `n` (1..`W5_LUCES`) on or off; lights revert when the app unloads. |
| 138 | `int ap_luz_estado(int n)` | Return 1 if light `n` is on, 0 if off, `W5_NO_DISPONIBLE` if absent or not responding. |

### Phone folders (1.2)

| # | function | what it does |
|---|---|---|
| 78 | `int fs_listar(const char *dir, unsigned desde, W5Ent *e, unsigned max)` | List any folder (read-only): COPIES up to `max` entries starting at entry `desde` into `e`; returns how many. |
| 116 | `int fs_contar(const char *dir)` | Return the number of entries in any folder (read-only); `W5_NO_DISPONIBLE` if it does not exist. |

### Music service (1.2)

| # | function | what it does |
|---|---|---|
| 93 | `int mu_abrir(void)` | Start the music service if not running (scans tracks, restores saved state); returns the track count. |
| 94 | `int mu_buscar(void)` | Rebuild the track list; returns the track count. |
| 95 | `int mu_info(W5MuInfo *m)` | COPIES the music service state into `m`; `W5_ERROR` if `m` does not belong to the app. |
| 96 | `int mu_pista(int i, char *ruta, unsigned max)` | COPIES the path of track `i` into `ruta` (NULL: type only); returns `W5_MU_T_*` or `W5_ERROR`. |
| 97 | `int mu_nombre(int i, char *buf, unsigned max)` | COPIES the name of track `i`, without folder or extension; returns its length or `W5_ERROR`. |
| 98 | `int mu_tocar(int i)` | Play track `i` from the start: `W5_OK`, or `W5_ERROR` if it cannot be read as MIDI. |
| 99 | `void mu_alternar(void)` | Toggle play/pause (when stopped, plays the loaded track, or the first one). |
| 100 | `void mu_parar(void)` | Stop music playback. |
| 101 | `void mu_saltar(int d)` | Skip tracks: +1 next, -1 previous (more than 3 s into a track, -1 restarts it). |
| 102 | `int mu_volumen(int v)` | Set the music volume 0..`W5_MU_VOL_MAX`; `W5_ERROR` if out of range. |
| 103 | `int mu_modo(int m)` | Set the output mode (`W5_MU_MODO_*`); `W5_ERROR` if out of range. |
| 104 | `int mu_repetir(int r)` | Set the repeat mode (`W5_MU_REP_*`); `W5_ERROR` if out of range. |
| 105 | `void mu_pantalla(int abierta)` | HOLD: 1 = the Walkman screen is mounted (Walkman key toggles instead of opening, volume keys control music); released on unload. |

### Lights (1.2)

| # | function | what it does |
|---|---|---|
| 106 | `int lu_leer(W5Luces *l)` | COPIES the current lights state and settings into `l`; `W5_ERROR` if `l` does not belong to the app. |
| 107 | `int lu_ajuste(int luces, int color, int latido)` | Set all THREE light settings (lights on/off, color 0..`n_colores`-1, pulse on/off) and save them; `W5_ERROR` if invalid. |
| 108 | `int lu_color_nombre(int i, char *buf, unsigned max)` | COPIES the name of palette color `i` ("automático", "blanco"…) into `buf`; returns its length or `W5_ERROR`. |
| 109 | `int lu_casa(int i)` | Play built-in effect `i` (stopping any running one): `W5_OK`, `W5_NO_DISPONIBLE` (chip not responding, screen off) or `W5_ERROR`. |
| 110 | `int lu_casa_nombre(int i, char *buf, unsigned max)` | COPIES the name of built-in effect `i` into `buf`; returns its length or `W5_ERROR`. |
| 111 | `int lu_fabricas(char *buf, unsigned max)` | COPIES the factory effect names (`contacteffects/X.ble`, `systemeffects/Y.ble`), each 0-terminated; returns the count, or `W5_NO_DISPONIBLE` without the original FS. |
| 112 | `int lu_fabrica(const char *relativo)` | Play a factory effect by relative name: `W5_OK`, `W5_NO_DISPONIBLE` (missing or chip not responding) or `W5_ERROR` (not a valid effect). |
| 113 | `void lu_parar(void)` | Stop the running light effect. |
| 114 | `int lu_sonando(char *buf, unsigned max)` | COPIES the name of the running effect (empty = none) into `buf`; returns its length. |
| 115 | `int lu_mantener(void)` | Keep the screen from dimming while an effect plays (counts as activity); no-op without an effect; returns 1 if there was one. |

### My files (1.2)

| # | function | what it does |
|---|---|---|
| 117 | `int ma_estado(W5MaEstado *e)` | Fill `e` with the My Files state: mounted, writable, free space, file count; returns `W5_OK`. |
| 118 | `int ma_listar(unsigned desde, W5Ent *e, unsigned max)` | COPIES the My Files entries, in name order, into `e`; returns how many. |
| 119 | `int ma_tam(const char *nombre)` | Return the size in bytes of a My Files file, or `W5_NO_DISPONIBLE`. |
| 120 | `int ma_leer(const char *nombre, unsigned desde, void *buf, unsigned max)` | Read a My Files file into `buf`: returns bytes read; `W5_NO_DISPONIBLE`; `W5_ERROR` (damaged). |
| 121 | `int ma_escribir(const char *nombre, const void *d, unsigned n)` | Write a My Files file whole and atomically; only `.txt` or the file the app was opened with; `W5_OK` or `W5_ERROR` (see `ma_motivo`). |
| 122 | `int ma_motivo(void)` | Return the `W5_MA_M_*` reason the last `ma_escribir` failed. |
| 123 | `const char * ma_motivo_texto(void)` | Return that failure reason as human-readable system text; valid for one frame. |

### Design system (1.3) and extra dialogs

| # | function | what it does |
|---|---|---|
| 124 | `void ui_pedir_nombre(const char *titulo, const char *inicial, W5Nombre hecho, void *dato)` | Show the system screen that asks for a file name; calls `hecho` with the name. |
| 139 | `W5W ui_fila_de(const W5FilaDatos *f, int sel)` | Build a list row with icon, title, subtitle and accessory from a `W5FilaDatos`; `sel` marks it selected. |
| 140 | `W5W ui_boton(const char *texto, int prioridad, int enfocado)` | Build a button of priority `W5_PRIO_*`, for the builder of an `Enfocable` widget. |
| 141 | `W5W ui_tarjeta(W5W hijo, int enfocada)` | Build a card: a grouped block of content around `hijo`. |
| 142 | `W5W ui_chip(const char *texto, int elegido, int enfocado)` | Build a chip: a compact option (filter) that can be selected and focused. |
| 143 | `W5W ui_control(int tipo, int activo, int enfocado)` | Build a `W5_CTL_*` control: switch, checkbox or radio option. |
| 144 | `W5W ui_deslizador(int valor, int max, int enfocado)` | Build a slider showing `valor` in 0..`max`; the app moves it with left/right keys. |
| 145 | `void ui_aviso_breve(const char *texto)` | Show a brief strip at the bottom that disappears after 3 s without taking focus. |
| 146 | `unsigned ui_token(int clave)` | Return a `W5_TK_*` design token: a color (0x00RRGGBB) or measure of the active design; 0 if it does not exist. |
| 147 | `int ui_diseno(char *buf, unsigned max)` | COPIES the name of the active design ("material3", "clasico") into `buf`; returns its length. |
| 148 | `int ui_diseno_pedir(const char *nombre)` | Request a specific design while the app is loaded: `W5_OK`, or `W5_NO_DISPONIBLE` (default stays); NULL releases it. |
| 149 | `W5W ui_tecla_boton(const char *texto, int rol, int elegida)` | Build an on-screen keyboard key that fills its box, with a `W5_TECLA_*` role. |

### Text input (extra)

| # | function | what it does |
|---|---|---|
| 125 | `int et_poner_modo(W5Et e, int modo)` | Set the writing mode (`W5_ET_*`) of text input `e`; returns `W5_OK`. |

### Notes store (1.2)

| # | function | what it does |
|---|---|---|
| 126 | `int nt_leer(int hueco, void *buf, unsigned max)` | Read note slot `hueco` (`W5_NT_ORDEN` = the order, 0..`W5_NT_N`-1 = a note): COPIES and returns its size; `W5_NO_DISPONIBLE` if absent. |
| 127 | `int nt_guardar(int hueco, const void *d, unsigned n)` | Save a note slot: `W5_OK`; `W5_ERROR` (does not fit, flash failure, rate limit); `W5_NO_DISPONIBLE` (no store or no write). |
| 128 | `int nt_olvidar(int hueco)` | Delete a note slot: `W5_OK`; `W5_NO_DISPONIBLE` (did not exist); `W5_ERROR`. |
| 129 | `int nt_escribible(void)` | Return 1 if the note store persists across power-off, 0 if not. |

### Images (1.2)

| # | function | what it does |
|---|---|---|
| 130 | `int im_abrir(const char *ruta)` | Decode an image from `ruta`: returns a handle > 0, or a `W5_IM_E_*` error. |
| 131 | `int im_info(int im, W5ImInfo *i)` | COPIES the open image's info into `i`: `W5_OK`, or `W5_ERROR` if the handle is not live. |
| 132 | `int im_siguiente(int im)` | Advance an animated GIF by one frame and return its duration in ms; 0 = not animated. |
| 133 | `void im_pintar(W5Lienzo l, int im, int x, int y)` | Inside a paint callback: draw image `im` with its corner at (`x`, `y`). |
| 134 | `void im_cerrar(int im)` | Release image `im` (any left open are released when the app unloads). |

### Sound and vibrator (1.5)

| # | function | what it does |
|---|---|---|
|  | `int mu_tocar_ruta(const char *ruta)` | Play any MIDI or MP3 on the phone by path via the music service; keeps playing with the app closed; needs `W5_CAP_MUSICA`. |
|  | `int ap_vibrar_patron(int hz, int pct, int ms)` | Vibrate for up to 10 s: continuous (`pct` 100) or pulsed (1/2/4/8 Hz at 25/50/75 %); `ms` <= 0 stops it. |

### Network (1.6)

| # | function | what it does |
|---|---|---|
|  | `int red_conectar(const unsigned char *addr)` | Start the network: radio, SPP link to the bridge, handshake. `addr` = 0: the one chosen in Settings. `W5_OK` / `W5_ERROR` (see `red_fallo`) / `W5_NO_DISPONIBLE`. |
|  | `void red_desconectar(void)` | Close everything (the radio stays on). |
|  | `int red_estado(void)` | Return the network state (`W5_RED_*`). |
|  | `int red_fallo(void)` | Return the reason for failure (`W5_REDF_*`). |
|  | `int red_http(const char *metodo, const char *url, const char *cabeceras, const void *cuerpo, unsigned n, unsigned banderas)` | Start an HTTP request; returns a channel or a `W5_RED_E_*` error. Headers are "Name: value\r\n"; everything is COPIED. |
|  | `int red_tcp(const char *host, unsigned puerto, unsigned banderas)` | Open a TCP connection to `host`:`puerto`; returns a channel or a `W5_RED_E_*` error. |
|  | `int red_canal(int c)` | Return the state (`W5_RC_*`) of channel `c`. |
|  | `int red_codigo(int c)` | Return how channel `c` ended (`W5_RCOD_*`, valid with `W5_RC_CERRADO`). |
|  | `int red_texto(int c, char *d, unsigned max)` | COPIES the reason the channel closed (e.g. "Name or service not known") into `d`; returns the bytes. |
|  | `int red_http_estado(int c)` | Return the HTTP status of channel `c` (200, 404…); 0 = not yet. |
|  | `int red_cabeceras(int c, char *d, unsigned max)` | COPIES the response headers of channel `c` into `d` (TCP: "ip:port"); returns the bytes. |
|  | `int red_fin_recibido(int c)` | Return 1 if the other side will send no more; whatever is still queued is the last data. |
|  | `int red_disponibles(int c)` | Return the bytes waiting in the receive queue of channel `c`. |
|  | `int red_recibir(int c, void *d, unsigned max)` | COPIES and removes received bytes from channel `c`; non-blocking (0 = nothing right now). |
|  | `int red_enviar(int c, const void *d, unsigned n)` | Send bytes on channel `c`; returns bytes accepted (up to 4 KB per call, 16 KB queue); send the rest on a later pass. |
|  | `int red_pendiente(int c)` | Return the bytes of channel `c` not yet sent. |
|  | `int red_terminar(int c)` | Send FIN: no more data from me (end of a chunked HTTP body / TCP half-close). |
|  | `void red_cerrar(int c)` | Release channel `c` (aborts it if still open). ALWAYS call it when done. |

### Screen (1.7)

| # | function | what it does |
|---|---|---|
|  | `int ui_paso_linea(int marcas)` | Return the line pitch of multi-line text with these flags (24 with the body font). |
|  | `int ui_alto_texto(int escala, int marcas)` | Return the height of ONE text line at the given scale and flags (what a `Texto` widget occupies). |
|  | `int pantalla_tam(void)` | Return the size of the LOGICAL screen the app sees now, as (width << 16) \| height. |
|  | `int pantalla_giro(void)` | Return the rotation: 0 upright, 1 sideways with keypad on the right, 2 upside down, 3 keypad on the left. |

### Music over the network (1.8)

| # | function | what it does |
|---|---|---|
|  | `int mu_red_vaciar(void)` | Empty the network queue (the current song plays to its end or until stopped). `W5_OK` / `W5_NO_DISPONIBLE`. |
|  | `int mu_red_poner(const char *url, const char *titulo, const char *artista, unsigned dur_ms)` | Append a song (URL, title, artist, duration in ms) to the queue: returns its index >= 0, `W5_ERROR` or `W5_NO_DISPONIBLE`. |
|  | `int mu_red_tocar(int i)` | Play queue song `i`, replacing whatever was playing. `W5_OK` / `W5_ERROR` / `W5_NO_DISPONIBLE`. |
|  | `int mu_red_info(W5MuRed *i)` | COPIES the network-music state into `i`. `W5_OK` / `W5_ERROR`. |
|  | `int mu_red_pista(int i, W5MuRedPista *p)` | Fill `p` with the URL and texts of queue song `i`. `W5_OK` / `W5_ERROR`. |
|  | `void li_pixeles(W5Lienzo l, int x, int y, int w, int h, const void *rgb565)` | Draw a w×h block of RGB565 pixels (high byte first) on the canvas with its corner at (`x`, `y`). |

### Camera (1.9)

| # | function | what it does |
|---|---|---|
|  | `int cam_abrir(void)` | Power on and program the sensor (~0.75 s). `W5_OK` (also if already open) / `W5_NO_DISPONIBLE` / `W5_ERROR` (see `cam_info` `.error`). |
|  | `int cam_cerrar(void)` | Turn the camera off. `W5_OK`. |
|  | `int cam_cuadro(void *dst, int formato)` | Fetch a frame: 1 = a NEW frame copied to `dst` (`W5_CAM_BYTES`); 0 = none yet; `W5_ERROR` / `W5_NO_DISPONIBLE`. |
|  | `int cam_info(W5CamInfo *i)` | COPIES the camera info into `i`. `W5_OK` / `W5_ERROR`. |
|  | `int cam_foto_guardar(const void *jpeg, unsigned n, char *nombre)` | Save a JPEG as `DSCnnnnn.JPG` in `W5_CAM_CARPETA` with the NEXT number; its name goes to `nombre[16]`. `W5_OK` / `W5_NO_DISPONIBLE` (no card) / `W5_ERROR`. |
|  | `int cam_foto_borrar(const char *nombre)` | Delete `nombre` from `W5_CAM_CARPETA`; ONLY a "DSCnnnnn.JPG". `W5_OK` / `W5_ERROR`. |

### Microphone (1.10)

| # | function | what it does |
|---|---|---|
|  | `int mic_abrir(void)` | START preparing the microphone (does not wait). `W5_OK` (also if already open) / `W5_NO_DISPONIBLE` / `W5_ERROR` (dictation holds it). |
|  | `int mic_cerrar(void)` | Turn the microphone off (music resumes if it was stopped). `W5_OK`. |
|  | `int mic_tomar(short *dst, unsigned max)` | Copy the accumulated samples to `dst` without waiting; returns the count (0 = nothing yet), or `W5_ERROR` (closed or invalid `dst`). |
|  | `int mic_info(W5MicInfo *i)` | COPIES the microphone info into `i`. `W5_OK` / `W5_ERROR`. |
|  | `int mic_wav_guardar(const short *pcm, unsigned n, char *nombre)` | Save `n` samples (up to `W5_MIC_WAV_MAX`) to `W5_MIC_CARPETA` as "Grabación (N).wav"; name to `nombre[32]`. `W5_OK` / `W5_NO_DISPONIBLE` / `W5_ERROR`. |

### Downloads (1.11)

| # | function | what it does |
|---|---|---|
|  | `int mu_red_descargar(int i)` | Download queue song `i` to `W5_DESC_CARPETA` as "Artist - Title.mp3"; the SYSTEM fetches it, even with the app closed. `W5_OK` / `W5_ERROR` / `W5_NO_DISPONIBLE`. |
|  | `int mu_red_descarga(W5MuDescarga *d)` | COPIES the state of the current or last download into `d`. `W5_OK` / `W5_ERROR`. |
|  | `int mu_red_descarga_cancelar(void)` | Cancel the download; nothing is left half-written on the card. `W5_OK`. |
