/* w5_microfono.h — EL MICRÓFONO PARA APPS (.sen): la ampliación 1.10 de la tabla `w5` (docs/es/sdk-api.md). El sistema la repite en su propia cabecera interna (el árbol del sistema comprueba que las dos dicen lo mismo).
 *
 * LO QUE HAY: el micrófono del W580: PCM de 16 bits con signo, MONO, a 8000 muestras por segundo (W5_MIC_HZ). SIN ESPERAS dentro de la app:
 *   mic_abrir EMPIEZA (el DSP tarda ~2-3 s en estar listo: mic_info dice PREPARANDO y luego GRABANDO); desde ese momento el SISTEMA recoge las
 *   muestras en su anillo (~2 s) y la app las va TOMANDO con mic_tomar en su ui_cada (cada 50-200 ms). Si tarda más de ~2 s en tomarlas, las
 *   nuevas se pierden (mic_info.perdidas).
 * LA MÚSICA: el micrófono usa el DSP y la música también: si suena música por el DSP (un MP3, «YT Music»), mic_abrir la PARA y mic_cerrar la
 *   vuelve a poner donde iba (mic_info.musica = 1). Si la app se va sin mic_cerrar, lo cierra el sistema.
 * GUARDAR: mic_wav_guardar escribe un WAV en la tarjeta, en la carpeta de las grabaciones del original (W5_MIC_CARPETA) con su estilo de nombre
 *   («Grabación (8).wav», el número siguiente). La app no puede escribir en otro sitio de la tarjeta.
 *
 *     W5_APP(.nombre = "Grabadora", .api_menor = W5_MIC_MENOR, .caps = W5_CAP_MICROFONO, …);   // sin W5_CAP_MICROFONO en .caps: opcional (w5_microfono_hay)
 *     iniciar:  if (w5_microfono_hay() && mic_abrir() == W5_OK) ui_cada(c, 100, cada);
 *     cada:     int n = mic_tomar(buf + llevo, MAX - llevo); if (n > 0) llevo += n;      // W5MicInfo i; mic_info(&i): i.estado, i.pico (para un vúmetro)
 *     desechar: mic_cerrar();
 * Ejemplo: senew-app-grabadora. No hay altavoz para reproducir PCM desde una app (el DSP no lo acepta aún): para oírlo, la tarjeta. */
#ifndef W5_MICROFONO_H
#define W5_MICROFONO_H
#include "w5.h"

#define W5_MIC_MENOR 10                        /* la menor de la API que trae este grupo */
#define W5_MIC_HZ 8000                         /* muestras por segundo: PCM de 16 bits con signo, mono */
#define W5_MIC_CARPETA "/tarjeta/music"        /* donde guarda mic_wav_guardar (la de las grabaciones del original); se lista con fs_listar */
#define W5_MIC_WAV_MAX (60 * W5_MIC_HZ)        /* lo más largo que guarda mic_wav_guardar: 60 s (960 KB) */
enum { W5_CAP_MICROFONO = 1 << 27 };           /* hay micrófono (el W580; en el banco de PC, uno de mentira) */
/* W5MicInfo.estado */
enum { W5_MIC_CERRADO = 0, W5_MIC_PREPARANDO, W5_MIC_GRABANDO, W5_MIC_FALLO };
typedef struct {
    unsigned char estado, mentira, musica, ocupado;   /* W5_MIC_*; 1 = el de mentira del PC; 1 = paró la música (vuelve al cerrar); 1 = lo tiene el dictado */
    int error;                                 /* el último fallo al prepararlo (< 0; 0 = ninguno) */
    unsigned muestras;                         /* las que ha tomado la app desde mic_abrir */
    unsigned desbordes, perdidas;              /* veces que el sistema llegó tarde al DSP (pudo perderse audio) y muestras tiradas porque nadie las tomaba */
    unsigned pico;                             /* el valor absoluto más alto de los últimos ~100-200 ms (0..32768): para un vúmetro */
    unsigned ms_preparar;                      /* lo que tardó en estar listo (0 mientras prepara) */
} W5MicInfo;

/* LAS MISMAS LÍNEAS que W5_API110 en la cabecera interna del sistema */
#define W5_API110(F) \
    F(int, mic_abrir, (void))                            /* EMPIEZA a prepararlo (no espera). W5_OK (también si ya estaba) / W5_NO_DISPONIBLE / W5_ERROR (lo tiene el dictado) */ \
    F(int, mic_cerrar, (void))                           /* lo apaga (y la música vuelve, si la paró). W5_OK */ \
    F(int, mic_tomar, (short *dst, unsigned max))        /* copia las muestras acumuladas, sin esperar: cuántas (0 = aún nada); W5_ERROR (cerrado o dst no vale) */ \
    F(int, mic_info, (W5MicInfo *i))                     /* la COPIA a i. W5_OK / W5_ERROR */ \
    F(int, mic_wav_guardar, (const short *pcm, unsigned n, char *nombre))   /* n muestras (≤ W5_MIC_WAV_MAX) a W5_MIC_CARPETA como «Grabación (N).wav»; su nombre a nombre[32]. W5_OK / W5_NO_DISPONIBLE / W5_ERROR */

/* Dónde va el grupo: detrás de la 1.3, del Bluetooth (1.4, 15), del sonido (1.5, 2), de la red (1.6, 18), de la pantalla (1.7, 4), de la música por la red
 * (1.8, 6) y de la cámara (1.9, 6). */
#define W5_MIC_CUENTA_(r, n, a) +1
enum { W5_MIC_N13 = 0 W5_API(W5_MIC_CUENTA_), W5_MIC_N14 = 15, W5_MIC_N15 = 2, W5_MIC_N16 = 18, W5_MIC_N17 = 4, W5_MIC_N18 = 6, W5_MIC_N19 = 6,
       W5_MIC_N110 = 0 W5_API110(W5_MIC_CUENTA_) };
#define W5_MIC_INDICE (W5_MIC_N13 + W5_MIC_N14 + W5_MIC_N15 + W5_MIC_N16 + W5_MIC_N17 + W5_MIC_N18 + W5_MIC_N19)   /* casilla de mic_abrir en la tabla */

typedef struct W5ApiMic {
    W5Api base;
    void (*antes_[W5_MIC_N14 + W5_MIC_N15 + W5_MIC_N16 + W5_MIC_N17 + W5_MIC_N18 + W5_MIC_N19])(void);
#define W5_MIC_F_(r, nombre, args) r (*nombre) args;
    W5_API110(W5_MIC_F_)
#undef W5_MIC_F_
} W5ApiMic;
#define W5MIC ((const W5ApiMic *)w5)

/* ¿están las funciones en ESTE sistema (tabla larga y menor >= 10) y hay micrófono? */
static inline int w5_microfono_hay(void)
{
    int api;
    if (!w5 || w5->n < (unsigned)(W5_MIC_INDICE + W5_MIC_N110)) return 0;
    api = ap_consulta(W5_Q_API);
    if (api < 0 || ((api >> 16) & 0xFFFF) != W5_API_MAYOR || (api & 0xFFFF) < W5_MIC_MENOR) return 0;
    return (ap_consulta(W5_Q_CAPS) & W5_CAP_MICROFONO) != 0;
}

#ifndef W5_MIC_SIN_NOMBRES
#define mic_abrir (W5MIC->mic_abrir)
#define mic_cerrar (W5MIC->mic_cerrar)
#define mic_tomar (W5MIC->mic_tomar)
#define mic_info (W5MIC->mic_info)
#define mic_wav_guardar (W5MIC->mic_wav_guardar)
#endif
#endif
