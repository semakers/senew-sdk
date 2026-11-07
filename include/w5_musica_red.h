/* w5_musica_red.h — LA MÚSICA POR LA RED Y LOS PÍXELES PARA APPS (.sen): la ampliación 1.8 de la tabla `w5` (docs/es/sdk-api.md). El sistema la repite en su propia cabecera interna (el árbol del sistema comprueba que las dos dicen lo mismo).
 *
 * LA COLA DE LA RED: la app pone canciones por su URL (un MP3 por http:// o https://, p. ej. las que sirve un servidor local del computador:
 * «http://127.0.0.1:PUERTO/pista/<id>.mp3») y elige cuál tocar. El SISTEMA las baja por la red del puente Bluetooth (1.6; no hace falta
 * red_conectar) a su colchón de 192 KB y las toca por el DSP: SIGUE SONANDO CON LA APP CERRADA y al acabar una pasa a la siguiente.
 * Lo demás es la música de siempre (1.2): mu_alternar (pausa/sigue), mu_saltar (±1 por la cola), mu_parar, mu_volumen, mu_info (estado,
 * volumen, pos y dur), y las teclas Play/Walkman/Vol±. Un corte del Bluetooth lo tapa el colchón (~12 s a 128 kbps); si dura más, pausa
 * limpia y sigue sola al volver la red, desde el byte que faltaba (el servidor tiene que entender «Range: bytes=N-»).
 *     W5_APP(.nombre = "Mi radio", .api_menor = W5_MURED_MENOR, .caps = W5_CAP_MUSICA | W5_CAP_GIRA, …);
 *     if (w5_mured_hay()) { mu_red_vaciar(); mu_red_poner("http://…/a.mp3", "Título", "Artista", 205000); mu_red_tocar(0); }
 *     … en un ui_cada: W5MuRed r; mu_red_info(&r);   // r.tocando == 0: «cargando…»; r.colchon_ms; r.porque si falla
 *
 * LI_PIXELES: un bloque w×h de píxeles RGB565 con el byte ALTO primero (como los da la red: p. ej. una portada servida por el computador) pintado
 * tal cual en el lienzo de una Pintura, recortado. No pide capacidad.
 *
 * CÓMO SE USA (como la red, es OPCIONAL): mu_red_* pide W5_CAP_MUSICA en .caps (como mu_tocar_ruta); antes de llamarlas, w5_mured_hay()
 * (tabla larga, menor >= 8 y el teléfono con red y DSP); li_pixeles, w5_pixeles_hay(). Ejemplo: senew-app-musica. */
#ifndef W5_MUSICA_RED_H
#define W5_MUSICA_RED_H
#include "w5.h"

#define W5_MURED_MENOR 8                       /* la menor de la API que trae este grupo */
#define W5_MURED_MAX 32                        /* canciones en la cola */
#define W5_MURED_URL 192                       /* bytes de una URL, con el 0 */
#define W5_MURED_TITULO 100                    /* bytes del título, con el 0 */
#define W5_MURED_ARTISTA 68                    /* bytes del artista, con el 0 */
/* W5MuRed.fase: en qué va la descarga de la canción actual */
enum { W5_MURED_QUIETA = 0, W5_MURED_CONECTANDO, W5_MURED_PIDIENDO, W5_MURED_RESPUESTA, W5_MURED_BAJANDO, W5_MURED_ENTERA, W5_MURED_FALLO };
typedef struct {
    short actual, n;                           /* la canción de la cola que suena o se baja (-1 = ninguna) y cuántas hay */
    unsigned char fase, red, reintentos, tocando;   /* W5_MURED_*; W5_RED_* de la red; cortes seguidos; 1 = ya suena (0 = aún cargando) */
    unsigned char activa, rsv[3];              /* 1 = lo que toca la música ahora viene de esta cola (mu_info: estado, volumen, pos y dur) */
    int http;                                  /* el último estado HTTP (200, 206, 404…; 0 = aún no) */
    unsigned recibidos, total;                 /* bytes de esta canción ya en el colchón y su tamaño (0 = no se sabe) */
    unsigned colchon, colchon_ms;              /* bytes en el colchón sin dar al DSP, y cuánto audio son (ms) */
    unsigned pos_ms, dur_ms;                   /* por dónde va y cuánto dura (0 = no se sabe) */
    unsigned esperas, ms_esperando;            /* pausas por falta de datos (el colchón se vació) y ms esperando */
    char porque[48];                           /* el último fallo, en palabras («el servidor dice HTTP 404», «la red: se cortó»…) */
} W5MuRed;
typedef struct {
    char url[W5_MURED_URL];
    char titulo[W5_MURED_TITULO];
    char artista[W5_MURED_ARTISTA];
    unsigned dur_ms;
} W5MuRedPista;

/* LAS MISMAS LÍNEAS que W5_API18 en la cabecera interna del sistema */
#define W5_API18(F) \
    F(int, mu_red_vaciar, (void))                        /* la cola vacía (lo que suena sigue hasta que acabe o se pare). W5_OK / W5_NO_DISPONIBLE */ \
    F(int, mu_red_poner, (const char *url, const char *titulo, const char *artista, unsigned dur_ms))   /* al final de la cola: índice >= 0, W5_ERROR, W5_NO_DISPONIBLE */ \
    F(int, mu_red_tocar, (int i))                        /* toca la canción i de la cola (lo que sonara, fuera). W5_OK / W5_ERROR / W5_NO_DISPONIBLE */ \
    F(int, mu_red_info, (W5MuRed *i))                    /* la COPIA a i. W5_OK / W5_ERROR */ \
    F(int, mu_red_pista, (int i, W5MuRedPista *p))       /* la URL y los textos de la canción i de la cola. W5_OK / W5_ERROR */ \
    F(void, li_pixeles, (W5Lienzo l, int x, int y, int w, int h, const void *rgb565))   /* w×h píxeles RGB565 (byte alto primero), con la esquina en (x, y) */

/* Dónde va el grupo: detrás de la 1.3, del Bluetooth (1.4, 15 funciones), del sonido (1.5, 2), de la red (1.6, 18) y de la pantalla (1.7, 4). */
#define W5_MURED_CUENTA_(r, n, a) +1
enum { W5_MURED_N13 = 0 W5_API(W5_MURED_CUENTA_), W5_MURED_N14 = 15, W5_MURED_N15 = 2, W5_MURED_N16 = 18, W5_MURED_N17 = 4,
       W5_MURED_N18 = 0 W5_API18(W5_MURED_CUENTA_) };
#define W5_MURED_INDICE (W5_MURED_N13 + W5_MURED_N14 + W5_MURED_N15 + W5_MURED_N16 + W5_MURED_N17)   /* casilla de mu_red_vaciar en la tabla */

typedef struct W5ApiMuRed {
    W5Api base;
    void (*antes_[W5_MURED_N14 + W5_MURED_N15 + W5_MURED_N16 + W5_MURED_N17])(void);
#define W5_MURED_F_(r, nombre, args) r (*nombre) args;
    W5_API18(W5_MURED_F_)
#undef W5_MURED_F_
} W5ApiMuRed;
#define W5MURED ((const W5ApiMuRed *)w5)

/* ¿están las funciones en ESTE sistema? (tabla larga y menor >= 8) */
static inline int w5_pixeles_hay(void)
{
    int api;
    if (!w5 || w5->n < (unsigned)(W5_MURED_INDICE + W5_MURED_N18)) return 0;
    api = ap_consulta(W5_Q_API);
    return api >= 0 && ((api >> 16) & 0xFFFF) == W5_API_MAYOR && (api & 0xFFFF) >= W5_MURED_MENOR;
}
/* ¿y además la música por la red? (el teléfono con radio: W5_CAP_RED; en el banco de PC no hay) */
#define W5_MURED_CAP_RED (1 << 24)             /* = W5_CAP_RED de w5_red.h (sin tener que incluirla) */
static inline int w5_mured_hay(void) { return w5_pixeles_hay() && (ap_consulta(W5_Q_CAPS) & W5_MURED_CAP_RED) != 0; }

#ifndef W5_MURED_SIN_NOMBRES
#define mu_red_vaciar (W5MURED->mu_red_vaciar)
#define mu_red_poner (W5MURED->mu_red_poner)
#define mu_red_tocar (W5MURED->mu_red_tocar)
#define mu_red_info (W5MURED->mu_red_info)
#define mu_red_pista (W5MURED->mu_red_pista)
#define li_pixeles (W5MURED->li_pixeles)
#endif
#endif
