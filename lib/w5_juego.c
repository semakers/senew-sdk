/* w5_juego.c — la parte del SDK de juegos que se enlaza en la app (api 1.1). Ver w5_juego.h. */
#include "w5_juego.h"
/* La firma va la PRIMERA en la ventana: w5_juego.ld pone .w5rec.firma delante de todo lo demás. */
__attribute__((used, section(".w5rec.firma"), aligned(16))) const unsigned w5_rec_firma[4] = { W5_REC_FIRMA, 0, 0, 0 };
extern const char w5_rec_ini[], w5_rec_fin[];             /* w5_juego.ld */
int w5_rec_en_ventana(void)
{
    const volatile unsigned *v = (const volatile unsigned *)W5_REC_VA;
    if (!(ap_consulta(W5_Q_CAPS) & W5_CAP_REC_VENTANA)) return 0;
    if ((unsigned)(w5_rec_fin - w5_rec_ini) > ap_rec_tam()) return 0;
    /* el contrato de los recursos sigue siendo ap_rec_mapa: con ventana, el mapa de todos los recursos ES W5_REC_VA */
    if (ap_rec_mapa(0, ap_rec_tam()) != (const void *)W5_REC_VA) return 0;
    return (const void *)w5_rec_firma == (const void *)W5_REC_VA && v[0] == W5_REC_FIRMA;
}
__asm__(".text\n .arm\n .align 2\n"
        " .global w5_guardar\n .type w5_guardar, %function\n"
        "w5_guardar:\n stmia r0, {r4-r11}\n str sp, [r0, #32]\n str lr, [r0, #36]\n mov r0, #0\n bx lr\n"
        " .global w5_saltar\n .type w5_saltar, %function\n"
        "w5_saltar:\n ldr sp, [r0, #32]\n ldr lr, [r0, #36]\n ldmia r0, {r4-r11}\n movs r0, r1\n moveq r0, #1\n bx lr\n");
