/* w5_sonido.h — LA API DE SONIDO Y VIBRADOR PARA APPS (.sen): la ampliación 1.5 de la tabla `w5` (docs/es/sdk-api.md).
 * Se incluye DESPUÉS de w5.h; no toca cargable.h. El sistema la repite en su propia cabecera interna (el árbol del
 * sistema comprueba que las dos dicen lo mismo).
 *
 *   mu_tocar_ruta(ruta)            toca CUALQUIER MIDI (.mid) o MP3 (.mp3, por el DSP) del teléfono por su ruta
 *                                  («/memoria/carefree.mp3», «/musica/x.mid»), por el servicio de música del sistema. Como lo
 *                                  que pone el Walkman, SIGUE SONANDO con la app cerrada; se para con mu_parar, se pausa con
 *                                  mu_alternar y su volumen es el de la música. Pide W5_CAP_MUSICA en .caps (como todo mu_*).
 *                                  W5_OK sonando · W5_NO_DISPONIBLE (no existe, no es MIDI ni MP3, o no se pudo arrancar) ·
 *                                  W5_ERROR (la ruta no es una cadena de la app, vacía o de más de 111 bytes).
 *   ap_vibrar_patron(hz, pct, ms)  el vibrador durante ms (como mucho W5_VIBRAR_MAX_MS = 10 s; más se recorta): CONTINUO con
 *                                  pct = 100, o A GOLPES: hz = 1, 2, 4 u 8 golpes por segundo, cada uno el 25, 50 o 75 % del
 *                                  periodo (se toma lo más cercano: 3 Hz y 60 % = 2 Hz al 50 %). ms <= 0 lo para. Se para solo al
 *                                  acabar el tiempo y al cerrarse la app. W5_OK · W5_NO_DISPONIBLE sin vibrador (mira
 *                                  W5_CAP_VIBRADOR) · W5_ERROR si hz no es 1..8 o pct no es 1..100.
 *                                  Ojo: el motor tarda ~50-100 ms en arrancar y en frenar; a 8 Hz y 25 % (31 ms) casi no se
 *                                  mueve. Medido en el W580i .
 *   LOS VOLÚMENES, con los ajustes de siempre (ap_ajuste / ap_ajuste_poner, 1.2), dos claves nuevas, 0..W5_VOL_MAX:
 *     W5_AJ_VOLUMEN          tonos y avisos del sistema (la fila «Volumen» de Ajustes; 0 = silencio)
 *     W5_AJ_VOLUMEN_MUSICA   la música (lo que mueven Vol+/Vol− mientras suena, y mu_volumen)
 *     ap_ajuste_poner(clave, v, guardar): guardar = 1 lo lleva ya a la flash; con 0 se guarda al cerrar la app. A diferencia
 *     de la alarma, NO piden W5_CAP_RELOJ.
 *
 * CÓMO SE USA (como el Bluetooth, es OPCIONAL: la app se compila con la API que diga cargable.h y carga en sistemas viejos):
 *   - antes de llamar a nada de esto: if (w5_son_hay()) … — mira que la tabla de ESTE sistema es lo bastante larga (w5->n) y
 *     que su menor es >= 5. En un sistema viejo las claves W5_AJ_VOLUMEN* contestan W5_ERROR.
 *   - para mu_tocar_ruta, poner W5_CAP_MUSICA en .caps de W5_APP (si no, el sistema cierra la app diciéndolo).
 *   - un ejemplo de todo: senew-app-sonido. */
#ifndef W5_SONIDO_H
#define W5_SONIDO_H
#include "w5.h"

#define W5_SON_MENOR 5                         /* la menor de la API que trae este grupo */
enum { W5_AJ_VOLUMEN = 5,                      /* 0..5: tonos y avisos del sistema (0 = silencio) */
       W5_AJ_VOLUMEN_MUSICA };                 /* 0..5: la música (el servicio de mu_*) */
#define W5_VOL_MAX 5
#define W5_VIBRAR_MAX_MS 10000                 /* lo más que dura una vibración pedida con ap_vibrar_patron */

#define W5_API15(F) \
    F(int, mu_tocar_ruta, (const char *ruta))            /* cualquier MIDI o MP3 del teléfono, por el servicio de música: sigue sonando con la app cerrada; pide W5_CAP_MUSICA */ \
    F(int, ap_vibrar_patron, (int hz, int pct, int ms))  /* continuo (pct 100) o a golpes (1/2/4/8 Hz al 25/50/75 %), hasta 10 s; ms <= 0 lo para */

/* Dónde va el grupo: detrás de la 1.3 (W5_API) y del grupo Bluetooth de la 1.4 (el grupo Bluetooth del sistema), que tiene W5_SON_N14 funciones.
 * No hace falta incluir nada más para usar esto: la casilla se cuenta aquí. */
#define W5_SON_CUENTA_(r, n, a) +1
enum { W5_SON_N13 = 0 W5_API(W5_SON_CUENTA_), W5_SON_N14 = 15, W5_SON_N15 = 0 W5_API15(W5_SON_CUENTA_) };
#define W5_SON_INDICE (W5_SON_N13 + W5_SON_N14)        /* casilla de mu_tocar_ruta en la tabla */

/* La tabla vista con el grupo: la de siempre, las 15 del Bluetooth (sin nombre aquí) y las de arriba. */
typedef struct W5ApiSon {
    W5Api base;
    void (*bt_[W5_SON_N14])(void);
#define W5_SON_F_(r, nombre, args) r (*nombre) args;
    W5_API15(W5_SON_F_)
#undef W5_SON_F_
} W5ApiSon;
#define W5SON ((const W5ApiSon *)w5)

/* ¿se puede llamar a esto en ESTE sistema? */
static inline int w5_son_hay(void)
{
    int api;
    if (!w5 || w5->n < (unsigned)(W5_SON_INDICE + W5_SON_N15)) return 0;
    api = ap_consulta(W5_Q_API);
    return api >= 0 && ((api >> 16) & 0xFFFF) == W5_API_MAYOR && (api & 0xFFFF) >= W5_SON_MENOR;
}

/* Los nombres cortos (mu_tocar_ruta() es W5SON->mu_tocar_ruta()), como w5_nombres.h. */
#ifndef W5_SON_SIN_NOMBRES
#define mu_tocar_ruta (W5SON->mu_tocar_ruta)
#define ap_vibrar_patron (W5SON->ap_vibrar_patron)
#endif
#endif
