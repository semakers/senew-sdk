/* hello.c -- the smallest useful senew app: a counter.
 *   OK       adds one          C       back to zero          right soft key (or Back)   leaves the app
 *
 * What it shows:
 *   - W5_APP(...)          the app's identity (name, version, API it needs); the packager reads it, it costs no RAM.
 *   - app_suceso()         the ONE function the system calls: W5_ABRIR returns the root screen.
 *   - ui_pantalla(...)     a screen = a "construir" function that returns a widget tree every time something changed.
 *   - Teclas / ui_andamio  key handling and the standard frame (title bar + soft-key labels).
 *   - ui_sucio()           "my state changed, build again".
 * Build:  make   ->  build/hello.sen   (upload it with SENUS: Apps -> "Subir .sen")
 */
#include "w5.h"
#include "w5_pantalla.h"     /* (API 1.7) the new font; also gives W5_PANT_MENOR */

W5_APP(.nombre = "Hello", .version = 1, .api_menor = W5_PANT_MENOR, .caps = W5_CAP_GIRA, .icono_gen = W5_IC_RELOJ);

static int count;            /* the app's state: plain statics (each launch starts cold) */

static int key(W5Ctx c, int k, void *dato)
{
    (void)dato;
    if (k == W5K_OK) count++;
    else if (k == W5K_C) count = 0;
    else return ui_teclas_defecto(c, k);   /* right soft key = back; everything else: system defaults */
    ui_sucio(ui_mi_pantalla(c));           /* state changed: build the screen again */
    return 1;                              /* key consumed */
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
    return 0;                              /* W5_PAUSA, W5_SIGUE, W5_SALIR: nothing to do here */
}
