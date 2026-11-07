/* w5_juego.h — lo del SDK para JUEGOS GRANDES y ports (api 1.1). Guía: docs/es/sdk.md.
 *
 *   - LA VENTANA DE RECURSOS: con .caps = W5_CAP_REC_VENTANA en W5_APP(...), enlazando con w5_juego.ld + el rec.ld
 *     que hace tools/sen_recursos.py, los datos de solo lectura grandes del juego viajan al final del .sen y el sistema
 *     los pone en W5_REC_VA con la MMU. El código del juego NO cambia: sus punteros ya son los de la ventana.
 *     w5_rec_en_ventana() comprueba, al empezar, que de verdad están ahí (la firma que w5_juego.c deja al principio).
 *   - SALIDA LIMPIA desde cualquier profundidad (el «pánico» de un port): w5_guardar(&s) en app_suceso y, desde donde
 *     sea, w5_saltar(&s, motivo != 0). Es un setjmp/longjmp de 10 palabras, sin libc.
 *   - Coma flotante y libm: el SDK enlaza con -lgcc (coma flotante por software, la de GCC); la libm va ESTÁTICA en la app
 *     (se enlaza como fuente en la propia app). La tabla no lleva nada de eso: un float
 *     no cruza nunca la tabla. */
#ifndef W5_JUEGO_H
#define W5_JUEGO_H
#include "w5.h"

#define W5_REC_FIRMA 0x31434552u                         /* «REC1», la primera palabra de la ventana */
extern const unsigned w5_rec_firma[4];                   /* { firma, bytes de recursos, 0, 0 }: w5_juego.c */
int w5_rec_en_ventana(void);                             /* 1 si los recursos están en W5_REC_VA y son los de esta app */

typedef struct { unsigned r[10]; } W5Salto;              /* r4-r11, sp, lr (ARM); en AArch64 no hay port de juegos */
int  w5_guardar(W5Salto *s) __attribute__((returns_twice));
void w5_saltar(W5Salto *s, int v) __attribute__((noreturn));
#endif
