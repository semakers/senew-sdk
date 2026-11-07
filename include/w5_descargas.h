/* w5_descargas.h — DESCARGAR CANCIONES DE LA RED A LA TARJETA (apps .sen): la ampliación 1.11 de la tabla `w5` (docs/es/sdk-api.md).
 * El sistema la repite en su propia cabecera interna (el árbol del sistema comprueba que las dos dicen lo mismo).
 *
 * Caso de uso: elegir una canción que está sonando y dejarla descargada en la tarjeta de memoria para poder oírla más tarde sin internet por Bluetooth.
 * La app pide descargar una canción de la COLA de la música por la red (1.8: mu_red_poner/mu_red_tocar); el SISTEMA la baja por su propio canal de la red
 * (mientras suena, o con la app cerrada) a la tarjeta, en la carpeta de música del original (W5_DESC_CARPETA), como «Artista - Título.mp3». Una a la vez;
 * una descarga cortada no deja nada a medias. Para oírla SIN red: mu_tocar_ruta (1.5) con su ruta; la Música del sistema también la ve.
 *     if (w5_desc_hay()) mu_red_descargar(r.actual);      … en un ui_cada: W5MuDescarga d; mu_red_descarga(&d);   // d.fase, d.bajados / d.esperado
 * Pide W5_CAP_MUSICA en .caps, como mu_red_*. Ejemplo: senew-app-musica (Opciones: «Descargar esta canción» y «Descargadas»). */
#ifndef W5_DESCARGAS_H
#define W5_DESCARGAS_H
#include "w5.h"

#define W5_DESC_MENOR 11                       /* la menor de la API que trae este grupo */
#define W5_DESC_CARPETA "/tarjeta/music"       /* donde quedan las canciones descargadas (la carpeta de música del original); se lista con fs_listar */
#define W5_DESC_NOMBRE 64                      /* el nombre del fichero con su 0 */
/* W5MuDescarga.fase */
enum { W5_DESC_QUIETA = 0, W5_DESC_CONECTANDO, W5_DESC_PIDIENDO, W5_DESC_RESPUESTA, W5_DESC_BAJANDO, W5_DESC_HECHA, W5_DESC_FALLO };
typedef struct {
    unsigned char fase, ya_estaba, rsv[2];     /* W5_DESC_*; 1 = ya estaba descargada (no se bajó otra vez) */
    int http;                                  /* el estado HTTP (200; 0 = aún no) */
    unsigned bajados, total, esperado;         /* bytes ya escritos, los del Content-Length (0 = no se sabe) y los que se esperan por la duración */
    char nombre[W5_DESC_NOMBRE];               /* «Artista - Título.mp3» (en W5_DESC_CARPETA) */
    char porque[48];                           /* el último fallo, en palabras */
} W5MuDescarga;

/* LAS MISMAS LÍNEAS que W5_API111 en la cabecera interna del sistema */
#define W5_API111(F) \
    F(int, mu_red_descargar, (int i))                    /* la canción i de la cola de la música por la red, a W5_DESC_CARPETA como «Artista - Título.mp3»: la baja el SISTEMA (sigue con la app cerrada). W5_OK (también si ya estaba) / W5_ERROR (otra en curso, o i no vale) / W5_NO_DISPONIBLE */ \
    F(int, mu_red_descarga, (W5MuDescarga *d))           /* la COPIA del estado de la descarga en curso o de la última. W5_OK / W5_ERROR */ \
    F(int, mu_red_descarga_cancelar, (void))             /* la deja (no queda nada a medias en la tarjeta). W5_OK */

/* Dónde va el grupo: detrás de la 1.3, del Bluetooth (1.4, 15), del sonido (1.5, 2), de la red (1.6, 18), de la pantalla (1.7, 4), de la música por la red
 * (1.8, 6), de la cámara (1.9, 6) y del micrófono (1.10, 5). */
#define W5_DESC_CUENTA_(r, n, a) +1
enum { W5_DESC_N13 = 0 W5_API(W5_DESC_CUENTA_), W5_DESC_N14 = 15, W5_DESC_N15 = 2, W5_DESC_N16 = 18, W5_DESC_N17 = 4, W5_DESC_N18 = 6, W5_DESC_N19 = 6,
       W5_DESC_N110 = 5, W5_DESC_N111 = 0 W5_API111(W5_DESC_CUENTA_) };
#define W5_DESC_INDICE (W5_DESC_N13 + W5_DESC_N14 + W5_DESC_N15 + W5_DESC_N16 + W5_DESC_N17 + W5_DESC_N18 + W5_DESC_N19 + W5_DESC_N110)

typedef struct W5ApiDesc {
    W5Api base;
    void (*antes_[W5_DESC_N14 + W5_DESC_N15 + W5_DESC_N16 + W5_DESC_N17 + W5_DESC_N18 + W5_DESC_N19 + W5_DESC_N110])(void);
#define W5_DESC_F_(r, nombre, args) r (*nombre) args;
    W5_API111(W5_DESC_F_)
#undef W5_DESC_F_
} W5ApiDesc;
#define W5DESC ((const W5ApiDesc *)w5)

/* ¿están las funciones en ESTE sistema (tabla larga y menor >= 11) y hay red? */
#define W5_DESC_CAP_RED (1 << 24)              /* = W5_CAP_RED de w5_red.h */
static inline int w5_desc_hay(void)
{
    int api;
    if (!w5 || w5->n < (unsigned)(W5_DESC_INDICE + W5_DESC_N111)) return 0;
    api = ap_consulta(W5_Q_API);
    if (api < 0 || ((api >> 16) & 0xFFFF) != W5_API_MAYOR || (api & 0xFFFF) < W5_DESC_MENOR) return 0;
    return (ap_consulta(W5_Q_CAPS) & W5_DESC_CAP_RED) != 0;
}

#ifndef W5_DESC_SIN_NOMBRES
#define mu_red_descargar (W5DESC->mu_red_descargar)
#define mu_red_descarga (W5DESC->mu_red_descarga)
#define mu_red_descarga_cancelar (W5DESC->mu_red_descarga_cancelar)
#endif
#endif
