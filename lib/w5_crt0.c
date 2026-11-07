/* w5_crt0.c — el arranque de una app cargable. El punto de entrada del .sen es _w5_entrada: guarda la tabla y
 * llama a app_suceso(). Y las cuatro funciones de memoria que el compilador puede llamar por su cuenta (al copiar
 * una estructura, al poner un arreglo a cero): van a la tabla. No hay nada más: ni constructores ni libc. */
#include "cargable.h"
const W5Api *w5;
W5Pal app_suceso(int suceso, W5Pal a, W5Pal b);
__attribute__((used, section(".w5entrada"))) W5Pal _w5_entrada(const W5Api *api, int suceso, W5Pal a, W5Pal b)
{
    if (suceso == W5_ABRIR) {
        if (!api || api->magia != W5_API_MAGIA || api->mayor != W5_API_MAYOR || api->tam_puntero != sizeof(void *)) return 0;
        w5 = api;
    }
    return app_suceso(suceso, a, b);
}
void *memcpy(void *d, const void *s, size_t n) { return w5->memcpy(d, s, n); }
void *memset(void *d, int c, size_t n) { return w5->memset(d, c, n); }
void *memmove(void *d, const void *s, size_t n) { return w5->memmove(d, s, n); }
int   memcmp(const void *a, const void *b, size_t n) { return w5->memcmp(a, b, n); }
