/* w5_pantalla.h — LA PANTALLA PARA APPS (.sen): la ampliación 1.7 de la tabla `w5` (docs/es/sdk-api.md).
 *
 * QUÉ ES LA 1.7: la letra del sistema creció (estética, 26-oct: cuerpo Roboto 20 px, línea de 24, filas de 28; antes cuerpo 10 px,
 * línea de 10, filas de 18). Una app que declara `.api_menor = W5_PANT_MENOR` en W5_APP dice «mido el texto»: el sistema le
 * pinta con la letra NUEVA. Por eso NO debe colocar nada con alturas de letra escritas a mano: las pregunta aquí.
 *
 *     W5_APP(.nombre = "Mi app", .api_menor = W5_PANT_MENOR, …);
 *     int linea = ui_paso_linea(0);                        // 24 con la letra nueva (10 con la de antes)
 *     int alto  = ui_alto_texto(2, 0);                     // lo que ocupa un Texto(…, A_ESCALA(2))
 *     int ancho = li_ancho_texto("Hola", 1, 0);            // (la de siempre, de la 1.0)
 *     int t = pantalla_tam(); int an = t >> 16, al = t & 0xFFFF;   // la pantalla lógica (240x320 en el W580)
 *
 * Una app de menor < 7 (todas las anteriores) corre en el MODO V1: el sistema le pone la letra y las medidas de ANTES, con las que
 * colocó sus cosas; sigue viéndose exactamente igual. Mejor que nada de esto es usar el layout (Columna, Fila, Expandido, ListaPerezosa
 * con A_ALTO_FILA(0) = el del diseño): se recoloca solo. */
#ifndef W5_PANTALLA_H
#define W5_PANTALLA_H
#include "w5.h"

#define W5_PANT_MENOR 7                        /* la menor de la API que trae este grupo: la letra nueva */
enum { W5_GIRO = 5 };                         /* suceso de app_suceso(): la pantalla giró (a = el giro nuevo, 0..3). Solo llega con W5_CAP_GIRA; las
                                                * pantallas de la app ya se vuelven a construir solas con el tamaño nuevo: es para quien pinta a mano */
enum { W5_CAP_GIRA = 1 << 25 };                /* la app SABE GIRAR: con la pantalla de lado o boca abajo se recoloca (pantalla_tam cambia).
                                                * Sin esto, mientras la app está cargada la pantalla va de pie */

/* LAS MISMAS LÍNEAS que W5_API17 en la cabecera interna del sistema */
#define W5_API17(F) \
    F(int, ui_paso_linea, (int marcas))                  /* el paso entre líneas de un texto de varias líneas con esas marcas (24 con el cuerpo) */ \
    F(int, ui_alto_texto, (int escala, int marcas))      /* el alto de UNA línea de texto con esa escala y marcas (lo que ocupa un Texto) */ \
    F(int, pantalla_tam, (void))                         /* (ancho << 16) | alto de la pantalla LÓGICA que ve la app ahora */ \
    F(int, pantalla_giro, (void))                        /* 0 de pie, 1 de lado con el teclado a la derecha, 2 boca abajo, 3 con el teclado a la izquierda */

/* Dónde va el grupo: detrás de la 1.3, del Bluetooth (1.4, 15 funciones), del sonido (1.5, 2) y de la red (1.6, 18). */
#define W5_PANT_CUENTA_(r, n, a) +1
enum { W5_PANT_N13 = 0 W5_API(W5_PANT_CUENTA_), W5_PANT_N14 = 15, W5_PANT_N15 = 2, W5_PANT_N16 = 18, W5_PANT_N17 = 0 W5_API17(W5_PANT_CUENTA_) };
#define W5_PANT_INDICE (W5_PANT_N13 + W5_PANT_N14 + W5_PANT_N15 + W5_PANT_N16)   /* casilla de ui_paso_linea en la tabla */

typedef struct W5ApiPant {
    W5Api base;
    void (*antes_[W5_PANT_N14 + W5_PANT_N15 + W5_PANT_N16])(void);
#define W5_PANT_F_(r, nombre, args) r (*nombre) args;
    W5_API17(W5_PANT_F_)
#undef W5_PANT_F_
} W5ApiPant;
#define W5PANT ((const W5ApiPant *)w5)

/* ¿se puede llamar a esto en ESTE sistema? (tabla larga y menor >= 7). Una app que declara .api_menor = 7 solo carga en un sistema
 * que lo tiene, así que para ella siempre da 1; sirve a la que quiera cargar también en uno viejo (declarando una menor más baja). */
static inline int w5_pantalla_hay(void)
{
    int api;
    if (!w5 || w5->n < (unsigned)(W5_PANT_INDICE + W5_PANT_N17)) return 0;
    api = ap_consulta(W5_Q_API);
    return api >= 0 && ((api >> 16) & 0xFFFF) == W5_API_MAYOR && (api & 0xFFFF) >= W5_PANT_MENOR;
}

#ifndef W5_PANT_SIN_NOMBRES
#define ui_paso_linea (W5PANT->ui_paso_linea)
#define ui_alto_texto (W5PANT->ui_alto_texto)
#define pantalla_tam (W5PANT->pantalla_tam)
#define pantalla_giro (W5PANT->pantalla_giro)
#endif
#endif
