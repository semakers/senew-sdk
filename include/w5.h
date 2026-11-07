/* w5.h — lo que incluye una APP CARGABLE del sistema («.sen»). SDK mínimo: esta cabecera, cargable.h (el contrato,
 * copia exacta de la del sistema), w5_nombres.h (generado), w5_crt0.c, w5_app.ld y mk/app.mk.
 * Una app NO incluye nada del sistema y no ve ningún símbolo suyo: todo va por la tabla `w5`.
 * Cómo se escribe una: docs/es/sdk.md y template/hello.c. */
#ifndef W5_H
#define W5_H
#include "cargable.h"
typedef unsigned char u8; typedef unsigned short u16; typedef unsigned int u32;
typedef signed short i16; typedef signed int i32;
#ifndef NULL
#define NULL ((void *)0)
#endif

void *memcpy(void *d, const void *s, size_t n);
void *memset(void *d, int c, size_t n);
void *memmove(void *d, const void *s, size_t n);
int   memcmp(const void *a, const void *b, size_t n);

extern const W5Api *w5;                 /* la tabla del sistema: la pone el crt0 antes de llamar a app_suceso() */
W5Pal app_suceso(int suceso, W5Pal a, W5Pal b);   /* LA función de la app: W5_ABRIR, W5_PAUSA, W5_SIGUE, W5_SALIR */

/* ---- los datos de la app para la cabecera del .sen (los lee el empaquetador; no ocupan memoria en el teléfono) ---- */
typedef struct {
    char magia[8];                      /* "W5META1" */
    char nombre[30];
    unsigned char perfil, icono_gen;
    unsigned short api_mayor, api_menor, an_min, al_min;
    unsigned caps, pila, monton, version;
    char icono[12][17];                 /* 12 filas de la misma longitud (1..16); «#» = tinta; vacío = sin icono propio */
} W5Meta;
#define W5_APP(...) \
    __attribute__((used, section(".w5meta"))) const W5Meta w5_meta = { .magia = "W5META1", .perfil = W5_PERFIL_WIDGETS, \
        .icono_gen = W5_IC_FICHERO, .api_mayor = W5_API_MAYOR, .api_menor = W5_API_MENOR, .pila = 8192, __VA_ARGS__ }

/* ---- los nombres cortos: ui_sucio(c) es w5->ui_sucio(c) ------------------------------------------------------------ */
#include "w5_nombres.h"

/* ---- los widgets: como en el sistema, pero con ATRIBUTOS en vez de campos --------------------------------------------
 *   Texto("hola", A_COLOR(c), A_ESCALA(2))      Columna(A_CRUZADO(W5P_ESTIRAR), HIJOS(a, b, c))      Caja(A_AN(40), A_HIJO(x)) */
#define W5_W(clase, ...) (ui_w((clase), (const W5Pal[]){ __VA_ARGS__ }, sizeof((W5Pal[]){ __VA_ARGS__ }) / sizeof(W5Pal) / 2))
#define HIJOS(...) W5A_HIJOS, (W5Pal)ui_lista((const W5W[]){ __VA_ARGS__ }, sizeof((W5W[]){ __VA_ARGS__ }) / sizeof(W5W))
#define A_CLAVE(v) W5A_CLAVE, (W5Pal)(v)
#define A_HIJO(v) W5A_HIJO, (W5Pal)(v)
#define A_AN(v) W5A_AN, (W5Pal)(v)
#define A_AL(v) W5A_AL, (W5Pal)(v)
#define A_COLOR(v) W5A_COLOR, (W5Pal)(v)
#define A_BORDE(v) W5A_BORDE, (W5Pal)(v)
#define A_FONDO(v) W5A_FONDO, (W5Pal)(v)
#define A_SOMBRA(v) W5A_SOMBRA, (W5Pal)(v)
#define A_BARRA(v) W5A_BARRA, (W5Pal)(v)
#define A_PRINCIPAL(v) W5A_PRINCIPAL, (W5Pal)(v)
#define A_CRUZADO(v) W5A_CRUZADO, (W5Pal)(v)
#define A_ENCOGE(v) W5A_ENCOGE, (W5Pal)(v)
#define A_ESPACIO(v) W5A_ESPACIO, (W5Pal)(v)
#define A_FLEX(v) W5A_FLEX, (W5Pal)(v)
#define A_COLUMNAS(v) W5A_COLUMNAS, (W5Pal)(v)
#define A_ALTO_CELDA(v) W5A_ALTO_CELDA, (W5Pal)(v)
#define A_ESCALA(v) W5A_ESCALA, (W5Pal)(v)
#define A_MARCAS(v) W5A_MARCAS, (W5Pal)(v)
#define A_MAX_LINEAS(v) W5A_MAX_LINEAS, (W5Pal)(v)
#define A_CENTRADO(v) W5A_CENTRADO, (W5Pal)(v)
#define A_VALOR(v) W5A_VALOR, (W5Pal)(v)
#define A_MAX(v) W5A_MAX, (W5Pal)(v)
#define A_MIN(v) W5A_MIN, (W5Pal)(v)
#define A_DATO(v) W5A_DATO, (W5Pal)(v)
#define A_VERSION(v) W5A_VERSION, (W5Pal)(v)
#define A_N(v) W5A_N, (W5Pal)(v)
#define A_AL_ACTIVAR(v) W5A_AL_ACTIVAR, (W5Pal)(v)
#define A_AL_MOVER(v) W5A_AL_MOVER, (W5Pal)(v)
#define A_SEL(v) W5A_SEL, (W5Pal)(v)
#define A_ALTO_FILA(v) W5A_ALTO_FILA, (W5Pal)(v)
#define A_AUTOFOCO(v) W5A_AUTOFOCO, (W5Pal)(v)
#define A_AL_CAMBIAR(v) W5A_AL_CAMBIAR, (W5Pal)(v)
#define A_DIGITOS(v) W5A_DIGITOS, (W5Pal)(v)
#define A_LINEAS(v) W5A_LINEAS, (W5Pal)(v)
#define A_AL_ENFOCAR(v) W5A_AL_ENFOCAR, (W5Pal)(v)
#define A_ANIM_MS(v) W5A_ANIM_MS, (W5Pal)(v)

#define Columna(...)          W5_W(W5C_FLEX, W5A_VERTICAL, 1 __VA_OPT__(,) __VA_ARGS__)
#define Fila(...)             W5_W(W5C_FLEX, W5A_VERTICAL, 0, W5A_CRUZADO, W5P_CENTRO __VA_OPT__(,) __VA_ARGS__)
#define Expandido(h, ...)     W5_W(W5C_EXPANDIDO, W5A_HIJO, (W5Pal)(h) __VA_OPT__(,) __VA_ARGS__)
#define Hueco()               W5_W(W5C_EXPANDIDO, W5A_FLEX, 1)
#define Relleno(n, h)         W5_W(W5C_RELLENO, W5A_HIJO, (W5Pal)(h), W5A_IZQ, (W5Pal)(n), W5A_ARR, (W5Pal)(n), W5A_DER, (W5Pal)(n), W5A_ABA, (W5Pal)(n))
#define RellenoXY(nx, ny, h)  W5_W(W5C_RELLENO, W5A_HIJO, (W5Pal)(h), W5A_IZQ, (W5Pal)(nx), W5A_ARR, (W5Pal)(ny), W5A_DER, (W5Pal)(nx), W5A_ABA, (W5Pal)(ny))
#define Centro(h)             W5_W(W5C_ALINEAR, W5A_HIJO, (W5Pal)(h))
#define Alinear(x, y, h)      W5_W(W5C_ALINEAR, W5A_HIJO, (W5Pal)(h), W5A_AX, (W5Pal)(x), W5A_AY, (W5Pal)(y))
#define Caja(...)             W5_W(W5C_CAJA __VA_OPT__(,) __VA_ARGS__)
#define Pila(...)             W5_W(W5C_PILA, HIJOS(__VA_ARGS__))
#define Rejilla(...)          W5_W(W5C_REJILLA __VA_OPT__(,) __VA_ARGS__)
#define Lista(...)            W5_W(W5C_LISTA __VA_OPT__(,) __VA_ARGS__)
#define Texto(s, ...)         W5_W(W5C_TEXTO, W5A_TEXTO, (W5Pal)(s) __VA_OPT__(,) __VA_ARGS__)
#define Parrafo(s, ...)       W5_W(W5C_PARRAFO, W5A_TEXTO, (W5Pal)(s) __VA_OPT__(,) __VA_ARGS__)
#define Velo(h)               W5_W(W5C_VELO, W5A_HIJO, (W5Pal)(h))
#define BarraDesp(total, visibles, pos, ...) W5_W(W5C_BARRA_DESP, W5A_TOTAL, (W5Pal)(total), W5A_VISIBLES, (W5Pal)(visibles), W5A_POS, (W5Pal)(pos) __VA_OPT__(,) __VA_ARGS__)
#define Progreso(valor, max, ...) W5_W(W5C_PROGRESO, W5A_VALOR, (W5Pal)(valor), W5A_MAX, (W5Pal)(max) __VA_OPT__(,) __VA_ARGS__)
#define Pintura(f, ...)       W5_W(W5C_PINTURA, W5A_PINTAR, (W5Pal)(f) __VA_OPT__(,) __VA_ARGS__)
#define ListaPerezosa(n, fila, ...) W5_W(W5C_LISTA_PEREZOSA, W5A_N, (W5Pal)(n), W5A_FILA, (W5Pal)(fila) __VA_OPT__(,) __VA_ARGS__)
#define Selector(pvalor, ...) W5_W(W5C_SELECTOR, W5A_VALOR, (W5Pal)(pvalor) __VA_OPT__(,) __VA_ARGS__)
#define CampoTexto(et, ...)   W5_W(W5C_CAMPO_TEXTO, W5A_ENTRADA, (W5Pal)(et) __VA_OPT__(,) __VA_ARGS__)
#define Icon(i, ...)          W5_W(W5C_ICONO, W5A_ICONO, (W5Pal)(i) __VA_OPT__(,) __VA_ARGS__)
#define Enfocable(constructor, ...) W5_W(W5C_ENFOCABLE, W5A_CONSTRUCTOR, (W5Pal)(constructor) __VA_OPT__(,) __VA_ARGS__)
#define Teclas(f, h, ...)     W5_W(W5C_TECLAS, W5A_AL_TECLA, (W5Pal)(f), W5A_HIJO, (W5Pal)(h) __VA_OPT__(,) __VA_ARGS__)
#endif
