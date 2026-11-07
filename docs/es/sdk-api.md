# Referencia de la API de apps `.sen`

> [!WARNING]
> **senew es un proyecto nuevo; úsalo bajo tu propio riesgo.** La API es estable por construcción (solo crece por el final), pero el sistema es joven y habrá fallos.

Esta referencia sale de las cabeceras de [`include/`](../../include) (`cargable.h` y `w5_*.h`), que son la fuente de verdad: si algo no cuadra, manda la cabecera.
Para una guía de principio a fin, lee antes [`sdk.md`](sdk.md); para apps que hablan con internet, [`apps-con-internet.md`](apps-con-internet.md).

## 1. Cómo funciona la API

- Una app **no enlaza con el sistema**. El sistema le pasa, al abrirla, un puntero a una **tabla de funciones** (`const W5Api *w5`), y la app llama a todo a través de ella.
  Las cabeceras definen nombres cortos para no escribir `w5->`: `ui_sucio(c)` es `w5->ui_sucio(c)`.
- La tabla tiene versión **MAYOR.MENOR**. Dentro de una mayor, **la tabla solo crece por el final** y ninguna función, atributo de widget, consulta o tecla cambia de número ni de significado.
  Una app declara la versión con que se compiló (`W5_APP(.api_menor = …)`): se carga si `mayor == la del sistema` y `menor <= la del sistema`; si pide más, el sistema dice «hace falta un sistema más nuevo».
- La base está en `cargable.h` (1.0 a 1.3, 150 funciones). Los grupos posteriores van en cabeceras aparte que se incluyen **detrás de `w5.h`** y son **opcionales** (una app que los use debe poder cargar en un sistema sin ellos, o declarar la API que necesita):
  `w5_sonido.h` (1.5), `w5_red.h` (1.6), `w5_pantalla.h` (1.7), `w5_musica_red.h` (1.8), `w5_camara.h` (1.9), `w5_microfono.h` (1.10), `w5_descargas.h` (1.11).
  Antes de llamar a un grupo opcional, pregunta si existe: `w5_red_hay()`, `w5_mured_hay()`, `w5_son_hay()`, `w5_pixeles_hay()`… (cada cabecera trae el suyo).
- **Nada del modelo de teléfono** está en la API: la pantalla (`W5_Q_ANCHO/W5_Q_ALTO`), las teclas que existen (`W5_Q_TECLAS_A/B`), las capacidades (`W5_Q_CAPS`) se **preguntan** con `ap_consulta()`. El mismo binario corre en una pantalla de 240×320 y en una de 176×220.
- **Ninguna estructura cruza la tabla por valor.** Por puntero cruzan unas pocas, fijadas campo a campo en `cargable.h` (`W5Rect`, `W5Fecha`, `W5Icono`, `W5Nota`…). Los widgets, el contexto, el lienzo y la entrada de texto son **manejadores opacos**.
- **ABI** (máquina 1, el teléfono): ARMv5TE, little-endian, AAPCS, coma flotante **por software** (no cruza la tabla), ARM o Thumb con interworking. Tipos: `char` 8, `short` 16, `int` 32, puntero = 32. Colores `0x00RRGGBB` (`W5_TRANSPARENTE` = «el del tema»).

## 2. Lo que el sistema le pide a la app

Una sola función: `W5Pal app_suceso(int suceso, W5Pal a, W5Pal b)`. Sucesos: `W5_ABRIR` (devuelve la pantalla raíz con `ui_pantalla(...)`, o 0 si no puede), `W5_PAUSA` / `W5_SIGUE` (deja de verse / vuelve; sin tics entre medias) y `W5_SALIR` (última ocasión de guardar).
Con el perfil `W5_PERFIL_JUEGO`, `W5_ABRIR` no vuelve hasta que el juego acaba (la app se queda con la pantalla y el bucle: `jg_*`).

## 3. Capacidades, consultas, teclas

- **Capacidades** (`W5_CAP_*`): lo que la app **necesita**; si el aparato no lo tiene, la app no se carga. Las opcionales no se piden: se preguntan con `ap_consulta(W5_Q_CAPS)`.
  `ACELEROMETRO, VIBRADOR, ZUMBADOR, LUCES, TAPA, FICHEROS, RELOJ, COLOR, TECLADO_NUM, JOYSTICK, REC_VENTANA, JIT, TECLAS_CRUDAS, GIRA` (la app sabe girar con la pantalla), `MUSICA, RED, IMAGENES, SERIE, PASOS, LUCES_EF`… (ver las cabeceras para la lista completa).
- **Consultas** (`ap_consulta`): `W5_Q_API, ANCHO, ALTO, CAPS, TECLAS_A, TECLAS_B, ALTO_LINEA, MODELO, PRIMER_PLANO, PILA_LIBRE, ACEL_1G, PIXEL, AVISOS`. Una que no existe devuelve −1.
- **Teclas** (`W5K_*`, códigos abstractos iguales en todos los modelos): `ARRIBA, ABAJO, IZQ, DER, OK, SEL_IZQ, SEL_DER, ATRAS, C, 0…9, AST (*), ALM (#), VOL_MAS, VOL_MENOS, WEB, PLAY, JUEGO_A, JUEGO_B, WALKMAN, CAMARA, BLOQUEO, ENCENDIDO`.
- **Códigos de retorno**: `W5_OK = 0`, `W5_NO_DISPONIBLE = -1` (no existe en este aparato / ahora no), `W5_ERROR = -2` (argumento malo). Los argumentos disparatados **cierran la app** con un mensaje, no la cuelgan ni cuelgan el teléfono.

## 4. Widgets

La interfaz se describe devolviendo un árbol de widgets desde una función `construir`. En el SDK se escriben con macros: `Columna(A_CRUZADO(W5P_ESTIRAR), HIJOS(a, b))`, `Texto("hola", A_COLOR(c), A_ESCALA(2))`.
Por dentro es `ui_w(clase, pares, n)` con parejas {atributo, valor}: lo que no se dice toma su valor por omisión; añadir clases y atributos no rompe apps viejas.

Clases: `Columna`/`Fila` (flex), `Expandido`, `Hueco`, `Relleno`, `Centro`/`Alinear`, `Caja`, `Pila`, `Rejilla`, `Lista`, `ListaPerezosa`, `Texto`, `Parrafo`, `Velo`, `BarraDesp`, `Progreso`, `Pintura` (pintas tú en un lienzo con `li_*`), `Selector`, `CampoTexto`, `Icon`, `Enfocable`, `Teclas`.
Atributos: `A_CLAVE, A_HIJO, A_AN, A_AL, A_COLOR, A_BORDE, A_FONDO, A_SOMBRA, A_BARRA, A_PRINCIPAL, A_CRUZADO, A_ENCOGE, A_ESPACIO, A_FLEX, A_COLUMNAS, A_ALTO_CELDA, A_ESCALA, A_MARCAS, A_MAX_LINEAS, A_CENTRADO, A_VALOR, A_MAX, A_MIN, A_DATO, A_VERSION, A_N, A_AL_ACTIVAR, A_AL_MOVER, A_SEL, A_ALTO_FILA, A_AUTOFOCO, A_AL_CAMBIAR, A_DIGITOS, A_LINEAS, A_AL_ENFOCAR, A_ANIM_MS`.
Definiciones exactas en `include/w5.h` y `include/cargable.h` (§6).

Un widget **dura un cuadro**: se construye en `construir()` y se devuelve; el estado vive en variables estáticas de la app. Para refrescar, `ui_sucio(ui_mi_pantalla(c))`. Los textos de `ui_fmt` también duran un cuadro: **cópialos** si los quieres guardar.
Los valores se validan según su tipo: un texto tiene que estar en memoria de la app, en la arena o en el sistema; un puntero a entero, dentro de la memoria de la app; una retrollamada, dentro de su código.

## 5. Límites

| Qué | Límite |
|---|---|
| Pila de la app | 4 KiB a 256 KiB (por omisión 8 KiB: `.pila` en `W5_APP`) |
| Montón (`ap_mem_pedir`, `.monton`) | hasta 14 MiB |
| Imagen (código + datos + bss) | hasta 8 MiB (API 1.1; 4 MiB en 1.0) |
| Reubicaciones | hasta 65 536 |
| Recursos de solo lectura (`--rec`) | hasta 16 MiB; se leen con `ap_rec_leer` / `ap_rec_mapa`, no ocupan RAM |
| Ficheros propios (`ap_fich_*`) | llevan el prefijo de la app; tope de 20 escrituras por minuto (desgaste de la flash) |
| Cálculos largos | el perro vigila unos 30 s por llamada a la app; `ap_latido()` lo renueva (hasta 60 s por llamada) |
| Apps a la vez | una |

Si la app falla (excepción de la CPU, cuelgue, pila desbordada, escribir junto a su memoria, punteros disparatados a la API), el sistema la cierra, dice por qué y sigue. Hay garantías que el hardware no puede imponer, así que algunas son parciales: **una app no es un sandbox perfecto**.

## 6. Tablas de funciones

Los números son las casillas de la tabla de `cargable.h` (las del 1.5 en adelante cuelgan de la 1.3 más el grupo Bluetooth, y no llevan número aquí). Los códigos `W5_*` están definidos en las cabeceras.

### Base

| n.º | función | qué hace |
|---|---|---|
| 0 | `void * memcpy(void *d, const void *s, size_t n)` |  |
| 1 | `void * memset(void *d, int c, size_t n)` |  |
| 2 | `void * memmove(void *d, const void *s, size_t n)` |  |
| 3 | `int memcmp(const void *a, const void *b, size_t n)` |  |
| 4 | `size_t strlen(const char *s)` |  |
| 5 | `int strcmp(const char *a, const char *b)` |  |

### La app y el aparato

| n.º | función | qué hace |
|---|---|---|
| 6 | `int ap_consulta(int que)` | W5_Q_*; -1 si esa consulta no existe |
| 7 | `void ap_salir(void)` | cierra la app (como «Atrás» en su pantalla raíz) |
| 8 | `void ap_latido(void)` | «sigo viva» dentro de un cálculo largo (como mucho 60 s por llamada) |
| 9 | `void * ap_mem_pedir(unsigned n)` | de SU montón; a cero; NULL si no cabe |
| 10 | `void ap_mem_soltar(void *p)` |  |
| 11 | `unsigned ap_mem_libre(void)` |  |
| 12 | `void ap_log(const char *s)` | traza de depuración |
| 13 | `unsigned ap_ms(void)` | milisegundos desde el arranque |
| 14 | `int ap_fecha(W5Fecha *f)` | hora local; W5_ERROR si el reloj no está en hora |
| 15 | `char * ap_hora_texto(int con_segundos)` | en el formato del usuario; dura un cuadro |

### Ficheros. Los de la app llevan SU prefijo, que pone el sistema: no ve los de nadie más

| n.º | función | qué hace |
|---|---|---|
| 16 | `int ap_fich_tam(const char *nombre)` | bytes, o W5_NO_DISPONIBLE |
| 17 | `int ap_fich_leer(const char *nombre, unsigned desde, void *buf, unsigned max)` |  |
| 18 | `int ap_fich_escribir(const char *nombre, const void *d, unsigned n)` | entero y atómico |
| 19 | `int ap_fich_borrar(const char *nombre)` |  |
| 20 | `int fs_tam(const char *ruta)` | cualquier fichero del teléfono, SOLO LECTURA |
| 21 | `int fs_leer(const char *ruta, unsigned desde, void *buf, unsigned max)` |  |

### Teclas, sensores, sonido (todo opcional: mira W5_Q_CAPS)

| n.º | función | qué hace |
|---|---|---|
| 22 | `int ap_tecla_abajo(int k)` | 1 si esa tecla está pulsada ahora |
| 23 | `int ap_acel(int encendido)` | pide o suelta el acelerómetro |
| 24 | `int ap_acel_leer(int *x, int *y)` | cuentas con signo, 0 = plano |
| 25 | `int ap_vibrar(int ms)` |  |
| 26 | `int ap_tono(unsigned hz, unsigned ms)` | se apaga solo |
| 27 | `int ap_sonar(int cual)` | W5_SN_*: con el volumen del usuario |
| 28 | `int ap_bateria(void)` | 0..100, o W5_NO_DISPONIBLE |

### Construir la interfaz

| n.º | función | qué hace |
|---|---|---|
| 29 | `W5W ui_w(unsigned clase, const W5Pal *pares, unsigned n)` |  |
| 30 | `W5W * ui_lista(const W5W *v, unsigned n)` | copia; quita los nulos; acaba en nulo |
| 31 | `char * ui_fmt(const char *f, ...)` | %d %u %x %s %c, con 0 y ancho; dura un cuadro |
| 32 | `void * ui_arena(unsigned n)` | memoria que dura un cuadro |
| 33 | `W5W ui_pantalla(W5Construir construir, W5Vida iniciar, W5Vida desechar, void *dato)` |  |
| 34 | `W5W ui_andamio(const char *titulo, W5W cuerpo, const char *izq, const char *der)` | barra de título + cuerpo + teclas de selección |
| 35 | `W5W ui_fila(const char *texto, const char *detalle, int sel)` |  |
| 36 | `W5W ui_dato(const char *nombre, const char *valor)` |  |
| 37 | `unsigned ui_color(int rol)` | W5_COL_*: la paleta del tema en uso |
| 38 | `void ui_sucio(W5Ctx c)` | «he cambiado: reconstrúyeme» |
| 39 | `W5Ctx ui_mi_pantalla(W5Ctx c)` | la pantalla de la app que contiene a c |
| 40 | `int ui_cada(W5Ctx c, unsigned ms, W5Retro f)` | mueren con c; no laten en pausa |
| 41 | `int ui_tras(W5Ctx c, unsigned ms, W5Retro f)` |  |
| 42 | `void ui_cancelar(W5Ctx c, W5Retro f)` |  |
| 43 | `void ui_empujar(W5W pantalla)` | otra pantalla de la app, encima |
| 44 | `void ui_sacar(void)` |  |
| 45 | `void ui_tecla(int k)` | entrega una tecla como si se pulsara |
| 46 | `int ui_teclas_defecto(W5Ctx c, int k)` | sel. izq. = OK, sel. der. / atrás = salir |
| 47 | `void ui_aviso(const char *titulo, const char *texto)` |  |
| 48 | `void ui_confirmar(const char *titulo, const char *texto, W5Si si, void *dato)` |  |
| 49 | `void ui_opciones(const char *titulo, const char *const *ops, int n, W5Elegir elegir, void *dato)` |  |

### Pintar

| n.º | función | qué hace |
|---|---|---|
| 50 | `void li_rect(W5Lienzo l, int x, int y, int w, int h, unsigned color)` |  |
| 51 | `void li_marco(W5Lienzo l, int x, int y, int w, int h, unsigned color)` |  |
| 52 | `void li_linea_h(W5Lienzo l, int x, int y, int w, unsigned color)` |  |
| 53 | `void li_punto(W5Lienzo l, int x, int y, unsigned color)` |  |
| 54 | `void li_texto(W5Lienzo l, int x, int y, const char *utf8, unsigned color, int escala, int marcas)` |  |
| 55 | `void li_texto_n(W5Lienzo l, int x, int y, const char *utf8, int bytes, unsigned color, int escala, int marcas)` |  |
| 56 | `int li_ancho_texto(const char *utf8, int escala, int marcas)` |  |
| 57 | `int li_partir(const char *utf8, int ancho, int escala, int marcas)` |  |
| 58 | `void li_velo(W5Lienzo l, int x, int y, int w, int h)` |  |
| 59 | `void li_icono(W5Lienzo l, int x, int y, const W5Icono *i, unsigned color, int escala)` |  |

### Entrada de texto

| n.º | función | qué hace |
|---|---|---|
| 60 | `W5Et et_nueva(unsigned cap)` | del montón de la app; NULL si no cabe |
| 61 | `void et_poner(W5Et e, const char *s)` |  |
| 62 | `const char * et_texto(W5Et e)` |  |
| 63 | `unsigned et_len(W5Et e)` |  |
| 64 | `int et_insertar(W5Et e, const char *utf8)` |  |
| 65 | `int et_borrar(W5Et e)` |  |
| 66 | `const char * et_modo(W5Et e)` | «abc», «Abc», «ABC», «123» |

### Recursos grandes de solo lectura (van al final del .sen; no ocupan RAM)

| n.º | función | qué hace |
|---|---|---|
| 67 | `unsigned ap_rec_tam(void)` |  |
| 68 | `int ap_rec_leer(unsigned desde, void *buf, unsigned max)` | bytes leídos |
| 69 | `const void * ap_rec_mapa(unsigned desde, unsigned n)` | puntero EN EL SITIO si el fichero está en flash mapeada y contiguo; si no, NULL |
| 70 | `int ap_rec_comprobar(void)` | recalcula el CRC de los recursos: W5_OK o W5_ERROR (lento) |

### PERFIL JUEGO: pantalla completa y bucle propio (solo apps con perfil W5_PERFIL_JUEGO)

| n.º | función | qué hace |
|---|---|---|
| 71 | `void * jg_tomar(int modo, int *an, int *al, int *paso)` | la pantalla: puntero al marco (W5_Q_PIXEL) o NULL |
| 72 | `void jg_volcar(void)` | el marco al panel; de paso, late |
| 73 | `void jg_soltar(void)` | devuelve la pantalla; después solo queda volver de W5_ABRIR |
| 74 | `void jg_espera(void)` | duerme hasta la siguiente interrupción (como mucho un tic) |
| 75 | `int jg_teclas(unsigned *a, unsigned *b)` | TODAS las teclas pulsadas ahora: bit k de a (k < 32) o de b; devuelve cuántas |
| 76 | `int jg_melodia(const W5Nota *notas, unsigned n, int repetir_desde)` | por el zumbador, la toca el sistema; NULL la calla |

### Servicios de la app (1.2): ajustes, hora, temporizador, retenciones, serie, teclas crudas, podómetro, fondo, JIT

| n.º | función | qué hace |
|---|---|---|
| 77 | `int ap_jit_sincronizar(const void *ini, unsigned n)` | W5_CAP_JIT: el código recién escrito en [ini, ini+n) de SU memoria ya se puede ejecutar |
| 79 | `int ap_ajuste(int clave)` | el valor de un ajuste W5_AJ_*; W5_ERROR si la clave no está en la lista |
| 80 | `int ap_ajuste_poner(int clave, int valor, int guardar)` | lo cambia (rango validado; W5_AJ_HORA12 no); guardar = 1 lo lleva ya a la flash |
| 81 | `int ap_hora_poner(const W5Fecha *f)` | pone el reloj en hora LOCAL (anio, mes, dia, hora, min, seg); W5_ERROR si la fecha no existe |
| 82 | `char * ap_zona_texto(void)` | la zona horaria del usuario, «UTC-4:00»; dura un cuadro |
| 83 | `int ap_temporizador_poner(unsigned ms)` | la cuenta atrás del SISTEMA vence dentro de ms (avisa aunque la app se cierre); 0 = pararla |
| 84 | `int ap_temporizador_queda(void)` | ms que le quedan, o W5_NO_DISPONIBLE si no hay cuenta atrás en marcha |
| 85 | `int ap_retener(int que, int si)` | W5_RET_*: pide (1) o suelta (0); al descargar se suelta todo |
| 86 | `int ap_pasos(W5Pasos *p)` | COPIA la foto del podómetro; W5_NO_DISPONIBLE sin W5_CAP_PASOS |
| 87 | `int ap_pasos_a_cero(void)` | el contador de pasos del sistema (el del standby) a cero |
| 88 | `int ap_serie_leer(void *buf, unsigned max)` | no bloquea: bytes recibidos (0 si nada); W5_NO_DISPONIBLE sin W5_CAP_SERIE |
| 89 | `int ap_serie_escribir(const void *d, unsigned n)` | bytes aceptados (0 sin cable: no cuelga) |
| 90 | `int ap_serie_conectado(void)` | 1 si hay algo al otro lado del cable |
| 91 | `int ap_teclas_mapa(unsigned char *col)` | COPIA 7 bytes: col[f] bit k = celda f*5+k pulsada (f = 6: teclas a masa); devuelve cuántas |
| 92 | `int ap_celda_nombre(int celda, char *buf, unsigned max)` | COPIA el nombre real de la celda 0..34 («Vol+»); «» si no hay tecla; su longitud |
| 135 | `int ap_poner_fondo(const char *ruta)` | el fondo del standby («» lo quita): W5_OK; W5_ERROR (no se pudo cargar); W5_NO_DISPONIBLE (no cabía: ya lo ha dicho el sistema) |
| 136 | `int ap_argumento(char *buf, unsigned max)` | con qué la abrió el sistema («» desde el menú): lo COPIA (acabado en 0) y da su longitud; W5_ERROR si buf no es suyo |
| 137 | `int ap_luz(int n, int encendida)` | W5_CAP_LUCES: la luz n (1..W5_LUCES) encendida o apagada; al descargar la app vuelven como estaban |
| 138 | `int ap_luz_estado(int n)` | 1 encendida, 0 apagada, W5_NO_DISPONIBLE si no hay o no contesta |

### Carpetas del teléfono (1.2)

| n.º | función | qué hace |
|---|---|---|
| 78 | `int fs_listar(const char *dir, unsigned desde, W5Ent *e, unsigned max)` | CUALQUIER carpeta, solo lectura: hasta max entradas desde la n.º desde, COPIADAS a e; cuántas |
| 116 | `int fs_contar(const char *dir)` | entradas de esa carpeta (cualquiera, solo lectura); W5_NO_DISPONIBLE si no existe |

### Servicio de música (1.2)

| n.º | función | qué hace |
|---|---|---|
| 93 | `int mu_abrir(void)` | arranca el servicio si no lo estaba (busca las pistas, recupera lo guardado); da cuántas pistas hay |
| 94 | `int mu_buscar(void)` | rehace la lista de pistas; da cuántas hay |
| 95 | `int mu_info(W5MuInfo *m)` | COPIA el estado del servicio; W5_ERROR si m no es de la app |
| 96 | `int mu_pista(int i, char *ruta, unsigned max)` | COPIA la ruta de la pista i (ruta NULL: solo el tipo); da W5_MU_T_*, o W5_ERROR |
| 97 | `int mu_nombre(int i, char *buf, unsigned max)` | COPIA su nombre sin carpeta ni extensión; da la longitud, o W5_ERROR |
| 98 | `int mu_tocar(int i)` | toca la pista i desde el principio: W5_OK, o W5_ERROR si no se deja leer como MIDI |
| 99 | `void mu_alternar(void)` | reproducir / pausa (parada: la cargada, o la primera) |
| 100 | `void mu_parar(void)` |  |
| 101 | `void mu_saltar(int d)` | +1 siguiente, -1 anterior (a más de 3 s de pista, vuelve a empezarla) |
| 102 | `int mu_volumen(int v)` | 0..W5_MU_VOL_MAX; W5_ERROR fuera de rango |
| 103 | `int mu_modo(int m)` | W5_MU_MODO_*; W5_ERROR fuera de rango |
| 104 | `int mu_repetir(int r)` | W5_MU_REP_*; W5_ERROR fuera de rango |
| 105 | `void mu_pantalla(int abierta)` | RETENCIÓN: 1 = la pantalla del Walkman está montada (la tecla Walkman alterna en vez de abrir; el volumen es de la música); se suelta sola al descargar la app |

### Luces (1.2)

| n.º | función | qué hace |
|---|---|---|
| 106 | `int lu_leer(W5Luces *l)` | COPIA las luces de ahora y los ajustes; W5_ERROR si l no es de la app |
| 107 | `int lu_ajuste(int luces, int color, int latido)` | los TRES ajustes de luces, con rango (luces y latido 0/1, color 0..n_colores-1); los guarda; W5_ERROR si no valen |
| 108 | `int lu_color_nombre(int i, char *buf, unsigned max)` | COPIA el nombre del color i de la paleta («automático», «blanco»…); da la longitud, o W5_ERROR |
| 109 | `int lu_casa(int i)` | reproduce el efecto de casa i (el que hubiera se para): W5_OK, W5_NO_DISPONIBLE (el chip no contesta, pantalla apagada), W5_ERROR |
| 110 | `int lu_casa_nombre(int i, char *buf, unsigned max)` | COPIA el nombre del efecto de casa i; da la longitud, o W5_ERROR |
| 111 | `int lu_fabricas(char *buf, unsigned max)` | COPIA los efectos de fábrica, «contacteffects/X.ble» y «systemeffects/Y.ble», uno tras otro, cada uno acabado en 0; da cuántos (los que quepan), o W5_NO_DISPONIBLE si no está el FS original |
| 112 | `int lu_fabrica(const char *relativo)` | reproduce uno de ellos (por su nombre relativo): W5_OK, W5_NO_DISPONIBLE (no está, o el chip no contesta), W5_ERROR (no es un efecto válido) |
| 113 | `void lu_parar(void)` | para el efecto en marcha |
| 114 | `int lu_sonando(char *buf, unsigned max)` | COPIA el nombre del efecto en marcha («» = ninguno); da la longitud |
| 115 | `int lu_mantener(void)` | mientras suena un efecto, que la pantalla no se atenúe (cuenta como actividad); sin efecto no hace nada; da 1 si lo había |

### Mis archivos (1.2)

| n.º | función | qué hace |
|---|---|---|
| 117 | `int ma_estado(W5MaEstado *e)` | Mis archivos: montado, escribible, libre, ficheros; W5_OK |
| 118 | `int ma_listar(unsigned desde, W5Ent *e, unsigned max)` | los ficheros de Mis archivos por orden de nombre, COPIADOS; cuántos |
| 119 | `int ma_tam(const char *nombre)` | bytes de ese fichero de Mis archivos, o W5_NO_DISPONIBLE |
| 120 | `int ma_leer(const char *nombre, unsigned desde, void *buf, unsigned max)` | bytes leídos; W5_NO_DISPONIBLE; W5_ERROR (dañado) |
| 121 | `int ma_escribir(const char *nombre, const void *d, unsigned n)` | entero y atómico; solo .txt o el fichero del argumento; W5_OK o W5_ERROR (ma_motivo) |
| 122 | `int ma_motivo(void)` | W5_MA_M_*: por qué falló la última ma_escribir |
| 123 | `const char * ma_motivo_texto(void)` | ese motivo dicho para una persona (las palabras del sistema); dura un cuadro |

### Sistema de diseño (1.3) y diálogos extra

| n.º | función | qué hace |
|---|---|---|
| 124 | `void ui_pedir_nombre(const char *titulo, const char *inicial, W5Nombre hecho, void *dato)` | la pantalla del sistema que pide un nombre de fichero |
| 139 | `W5W ui_fila_de(const W5FilaDatos *f, int sel)` | una fila de lista con icono, título, subtítulo y accesorio |
| 140 | `W5W ui_boton(const char *texto, int prioridad, int enfocado)` | W5_PRIO_*: para el constructor de un Enfocable |
| 141 | `W5W ui_tarjeta(W5W hijo, int enfocada)` | un bloque de contenido agrupado |
| 142 | `W5W ui_chip(const char *texto, int elegido, int enfocado)` | una opción compacta (filtro) |
| 143 | `W5W ui_control(int tipo, int activo, int enfocado)` | W5_CTL_*: interruptor, casilla u opción |
| 144 | `W5W ui_deslizador(int valor, int max, int enfocado)` | un valor en un rango (lo mueve la app con izq./der.) |
| 145 | `void ui_aviso_breve(const char *texto)` | una tira abajo que se va sola a los 3 s (no quita el foco) |
| 146 | `unsigned ui_token(int clave)` | W5_TK_*: el color (0x00RRGGBB) o la medida del diseño en uso; 0 si no existe |
| 147 | `int ui_diseno(char *buf, unsigned max)` | COPIA el nombre del diseño en uso («material3», «clasico»); su longitud |
| 148 | `int ui_diseno_pedir(const char *nombre)` | pide un diseño concreto mientras la app esté cargada: W5_OK, o W5_NO_DISPONIBLE (sigue el de serie); NULL = soltar |
| 149 | `W5W ui_tecla_boton(const char *texto, int rol, int elegida)` | una tecla de un teclado en pantalla que llena su caja: W5_TECLA_* |

### Entrada de texto (extra)

| n.º | función | qué hace |
|---|---|---|
| 125 | `int et_poner_modo(W5Et e, int modo)` | W5_ET_*: el modo de escritura de esa entrada; W5_OK |

### Almacén de notas (1.2)

| n.º | función | qué hace |
|---|---|---|
| 126 | `int nt_leer(int hueco, void *buf, unsigned max)` | hueco W5_NT_ORDEN = el orden, 0..W5_NT_N-1 = una nota: COPIA y da los bytes que mide; W5_NO_DISPONIBLE si no existe |
| 127 | `int nt_guardar(int hueco, const void *d, unsigned n)` | W5_OK; W5_ERROR (no cabe, fallo de flash, tope); W5_NO_DISPONIBLE (sin almacén o sin escritura) |
| 128 | `int nt_olvidar(int hueco)` | W5_OK; W5_NO_DISPONIBLE (no existía); W5_ERROR |
| 129 | `int nt_escribible(void)` | 1 = el almacén guarda (sobrevive a apagar); 0 = no |

### Imágenes (1.2)

| n.º | función | qué hace |
|---|---|---|
| 130 | `int im_abrir(const char *ruta)` | decodifica una imagen: manejador > 0, o W5_IM_E_* |
| 131 | `int im_info(int im, W5ImInfo *i)` | la COPIA a i; W5_OK o W5_ERROR (no es un manejador vivo) |
| 132 | `int im_siguiente(int im)` | avanza un cuadro de un GIF animado: sus ms; 0 = no está animada |
| 133 | `void im_pintar(W5Lienzo l, int im, int x, int y)` | dentro de una retrollamada de pintar: la imagen con su esquina en (x, y) |
| 134 | `void im_cerrar(int im)` | la suelta (las que no, se sueltan al descargar la app) |

### Sonido y vibrador (1.5)

| n.º | función | qué hace |
|---|---|---|
|  | `int mu_tocar_ruta(const char *ruta)` | cualquier MIDI o MP3 del teléfono, por el servicio de música: sigue sonando con la app cerrada; pide W5_CAP_MUSICA |
|  | `int ap_vibrar_patron(int hz, int pct, int ms)` | continuo (pct 100) o a golpes (1/2/4/8 Hz al 25/50/75 %), hasta 10 s; ms <= 0 lo para |

### Red (1.6)

| n.º | función | qué hace |
|---|---|---|
|  | `int red_conectar(const unsigned char *addr)` | arranca: radio + SPP con el puente + saludo. addr = 0: el elegido en Ajustes. W5_OK / W5_ERROR (mirar red_fallo) / W5_NO_DISPONIBLE |
|  | `void red_desconectar(void)` | cierra todo (la radio sigue encendida) |
|  | `int red_estado(void)` | W5_RED_* |
|  | `int red_fallo(void)` | W5_REDF_* |
|  | `int red_http(const char *metodo, const char *url, const char *cabeceras, const void *cuerpo, unsigned n, unsigned banderas)` | canal o W5_RED_E_*; cabeceras «Nombre: valor\r\n»…; todo se COPIA |
|  | `int red_tcp(const char *host, unsigned puerto, unsigned banderas)` | canal o W5_RED_E_* |
|  | `int red_canal(int c)` | W5_RC_* |
|  | `int red_codigo(int c)` | W5_RCOD_* (con W5_RC_CERRADO) |
|  | `int red_texto(int c, char *d, unsigned max)` | COPIA el porqué del cierre («Name or service not known»…); bytes |
|  | `int red_http_estado(int c)` | 200, 404…; 0 = aún no |
|  | `int red_cabeceras(int c, char *d, unsigned max)` | COPIA las cabeceras de la respuesta (TCP: «ip:puerto»); bytes |
|  | `int red_fin_recibido(int c)` | 1 = el otro lado ya no mandará más: lo que quede en la cola es lo último |
|  | `int red_disponibles(int c)` | bytes esperando en la cola de recepción |
|  | `int red_recibir(int c, void *d, unsigned max)` | COPIA y saca; no bloquea (0 = nada ahora) |
|  | `int red_enviar(int c, const void *d, unsigned n)` | bytes aceptados (hasta 4 KB por llamada; la cola del canal es de 16 KB); el resto, en otra vuelta |
|  | `int red_pendiente(int c)` | bytes aún sin salir |
|  | `int red_terminar(int c)` | FIN: no mandaré más (fin del cuerpo HTTP a trozos / medio cierre TCP) |
|  | `void red_cerrar(int c)` | soltar el canal (si seguía abierto, se aborta). SIEMPRE al acabar |

### Pantalla (1.7)

| n.º | función | qué hace |
|---|---|---|
|  | `int ui_paso_linea(int marcas)` | el paso entre líneas de un texto de varias líneas con esas marcas (24 con el cuerpo) |
|  | `int ui_alto_texto(int escala, int marcas)` | el alto de UNA línea de texto con esa escala y marcas (lo que ocupa un Texto) |
|  | `int pantalla_tam(void)` | (ancho << 16) \| alto de la pantalla LÓGICA que ve la app ahora |
|  | `int pantalla_giro(void)` | 0 de pie, 1 de lado con el teclado a la derecha, 2 boca abajo, 3 con el teclado a la izquierda |

### Música por la red (1.8)

| n.º | función | qué hace |
|---|---|---|
|  | `int mu_red_vaciar(void)` | la cola vacía (lo que suena sigue hasta que acabe o se pare). W5_OK / W5_NO_DISPONIBLE |
|  | `int mu_red_poner(const char *url, const char *titulo, const char *artista, unsigned dur_ms)` | al final de la cola: índice >= 0, W5_ERROR, W5_NO_DISPONIBLE |
|  | `int mu_red_tocar(int i)` | toca la canción i de la cola (lo que sonara, fuera). W5_OK / W5_ERROR / W5_NO_DISPONIBLE |
|  | `int mu_red_info(W5MuRed *i)` | la COPIA a i. W5_OK / W5_ERROR |
|  | `int mu_red_pista(int i, W5MuRedPista *p)` | la URL y los textos de la canción i de la cola. W5_OK / W5_ERROR |
|  | `void li_pixeles(W5Lienzo l, int x, int y, int w, int h, const void *rgb565)` | w×h píxeles RGB565 (byte alto primero), con la esquina en (x, y) |

### Cámara (1.9)

| n.º | función | qué hace |
|---|---|---|
|  | `int cam_abrir(void)` | enciende y programa el sensor (~0,75 s). W5_OK (también si ya estaba) / W5_NO_DISPONIBLE / W5_ERROR (cam_info.error) |
|  | `int cam_cerrar(void)` | la apaga. W5_OK |
|  | `int cam_cuadro(void *dst, int formato)` | 1 = un cuadro NUEVO copiado a dst (W5_CAM_BYTES); 0 = aún no hay; W5_ERROR / W5_NO_DISPONIBLE |
|  | `int cam_info(W5CamInfo *i)` | la COPIA a i. W5_OK / W5_ERROR |
|  | `int cam_foto_guardar(const void *jpeg, unsigned n, char *nombre)` | un JPEG (FF D8 … FF D9) a W5_CAM_CARPETA como DSCnnnnn.JPG, con el número SIGUIENTE (como el original); su nombre a nombre[16]. W5_OK / W5_NO_DISPONIBLE (sin tarjeta) / W5_ERROR |
|  | `int cam_foto_borrar(const char *nombre)` | borra W5_CAM_CARPETA/nombre; SOLO un «DSCnnnnn.JPG». W5_OK / W5_ERROR |

### Micrófono (1.10)

| n.º | función | qué hace |
|---|---|---|
|  | `int mic_abrir(void)` | EMPIEZA a prepararlo (no espera). W5_OK (también si ya estaba) / W5_NO_DISPONIBLE / W5_ERROR (lo tiene el dictado) |
|  | `int mic_cerrar(void)` | lo apaga (y la música vuelve, si la paró). W5_OK |
|  | `int mic_tomar(short *dst, unsigned max)` | copia las muestras acumuladas, sin esperar: cuántas (0 = aún nada); W5_ERROR (cerrado o dst no vale) |
|  | `int mic_info(W5MicInfo *i)` | la COPIA a i. W5_OK / W5_ERROR |
|  | `int mic_wav_guardar(const short *pcm, unsigned n, char *nombre)` | n muestras (≤ W5_MIC_WAV_MAX) a W5_MIC_CARPETA como «Grabación (N).wav»; su nombre a nombre[32]. W5_OK / W5_NO_DISPONIBLE / W5_ERROR |

### Descargas (1.11)

| n.º | función | qué hace |
|---|---|---|
|  | `int mu_red_descargar(int i)` | la canción i de la cola de la música por la red, a W5_DESC_CARPETA como «Artista - Título.mp3»: la baja el SISTEMA (sigue con la app cerrada). W5_OK (también si ya estaba) / W5_ERROR (otra en curso, o i no vale) / W5_NO_DISPONIBLE |
|  | `int mu_red_descarga(W5MuDescarga *d)` | la COPIA del estado de la descarga en curso o de la última. W5_OK / W5_ERROR |
|  | `int mu_red_descarga_cancelar(void)` | la deja (no queda nada a medias en la tarjeta). W5_OK |
