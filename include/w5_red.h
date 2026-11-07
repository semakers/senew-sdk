/* w5_red.h — LA RED PARA APPS (.sen): la ampliación 1.6 de la tabla `w5` (docs/es/sdk-api.md).
 * PARA EMPEZAR UNA APP CONECTADA: la guía docs/es/apps-con-internet.md (las reglas, recetas, cifras y fallos) y la plantilla
 * examples/plantilla_red/plantilla_red.c.
 *
 * El W580 no tiene WiFi ni TLS: la red le llega por BLUETOOTH desde un «puente» (la app de escritorio SENUS en el computador del usuario, mientras está abierta;
 * encendido, como un módem). El teléfono abre un puerto serie (SPP) con el puente y por él MULTIPLEXA canales; el trabajo de red de
 * verdad (DNS, TCP, TLS, HTTP) lo hace el puente. Así una app puede:
 *   - hacer PETICIONES HTTP con cualquier verbo (GET, POST, PUT, DELETE, PATCH, HEAD…), http:// y https://, con cabeceras y cuerpo;
 *   - abrir SOCKETS TCP (con TLS si se pide) y mandar y recibir un flujo de bytes en los dos sentidos.
 * Hasta W5_RED_CANALES a la vez. Cada canal tiene su cola de recepción (8 KB) con CONTROL DE FLUJO: si la app no lee, el otro lado
 * espera; no se pierde nada. Velocidad medida del enlace: ~150 KB/s por sentido .
 *
 * El Bluetooth es UNO A LA VEZ: o la red (por el puente) o un robot (bt_conectar_spp). Mientras la red está
 * conectada, bt_conectar_spp da W5_ERROR, y al revés red_conectar da W5_ERROR con fallo W5_REDF_OCUPADA.
 *
 * TODO ES ASÍNCRONO (como el Bluetooth): las órdenes arrancan algo y se mira cómo va en un ui_cada:
 *     if (w5_red_hay() && red_conectar(0) == W5_OK) …              // 0 = el puente elegido en Ajustes -> Bluetooth
 *     … cuando red_estado() == W5_RED_LISTA:
 *     int c = red_http("GET", "https://example.com/", 0, 0, 0, 0);  // canal (> 0) o un W5_RED_E_* (< 0)
 *     … y en cada vuelta: n = red_recibir(c, buf, sizeof buf);     // el cuerpo, a trozos
 *     red_http_estado(c) -> 200; red_canal(c) == W5_RC_CERRADO y red_disponibles(c) == 0 -> acabado (red_codigo(c) dice cómo)
 *     red_cerrar(c);                                                // SIEMPRE, para soltar el canal
 * La app es DUEÑA de la red mientras está cargada: al descargarla, el sistema cierra los canales y desconecta.
 *
 * CÓMO SE USA COMO «OPCIONAL» (como el Bluetooth): no poner W5_CAP_RED en .caps de W5_APP si la app debe cargar también donde no hay
 * red; y antes de llamar a nada de esto, if (w5_red_hay()) … */
#ifndef W5_RED_H
#define W5_RED_H
#include "w5.h"

#define W5_RED_MENOR 6                         /* la menor de la API que trae este grupo */
enum { W5_CAP_RED = 1 << 24 };                 /* hay red por el puente Bluetooth (el W580 con su radio) */
#define W5_RED_CANALES 4
/* red_estado() */
enum { W5_RED_APAGADA = 0, W5_RED_CONECTANDO, W5_RED_LISTA, W5_RED_FALLO };
/* red_fallo(): por qué está en W5_RED_FALLO */
enum { W5_REDF_OK = 0, W5_REDF_SIN_RADIO, W5_REDF_SIN_PUENTE /* no hay puente elegido (Ajustes -> Bluetooth) ni dirección */,
       W5_REDF_RADIO /* la radio no arrancó */, W5_REDF_ENLACE /* el puente no contesta, no tiene SPP o rechazó el PIN */,
       W5_REDF_SALUDO /* contestó, pero no es un puente W5RED */, W5_REDF_CAIDA /* se cortó estando conectada */,
       W5_REDF_OCUPADA /* el Bluetooth lo usa otra cosa (un robot conectado) */ };
/* red_canal(c) */
enum { W5_RC_LIBRE = 0, W5_RC_ABRIENDO, W5_RC_ABIERTO, W5_RC_CERRADO, W5_RC_CERRANDO };
/* red_codigo(c): cómo acabó (con W5_RC_CERRADO). W5_RCOD_BIEN = acabó bien (respuesta entera / el otro cerró) */
enum { W5_RCOD_BIEN = 0, W5_RCOD_DNS, W5_RCOD_RECHAZADA, W5_RCOD_PLAZO, W5_RCOD_TLS, W5_RCOD_HTTP, W5_RCOD_RED, W5_RCOD_CORTADA,
       W5_RCOD_PROHIBIDO, W5_RCOD_DEMASIADOS, W5_RCOD_PROTOCOLO, W5_RCOD_ABORTADA, W5_RCOD_INTERNO, W5_RCOD_SIN_SESION };
/* banderas de red_http / red_tcp; el PLAZO de conexión en segundos va en los bits 8..15 (0 = 30 s): W5_RED_PLAZO(10) */
enum { W5_RED_TLS = 1 /* solo TCP: con TLS (el puente comprueba el certificado) */, W5_RED_SIN_REDIR = 2 /* HTTP: no seguir 3xx */,
       W5_RED_SIN_VERIFICAR = 4 /* no comprobar el certificado TLS */ };
#define W5_RED_PLAZO(s) (((unsigned)(s) & 255u) << 8)
/* lo que pasa al abrir mal (negativo) */
enum { W5_RED_E_SIN_SESION = -1, W5_RED_E_OCUPADO = -2 /* reintentar en la vuelta siguiente */, W5_RED_E_ARG = -3,
       W5_RED_E_SIN_CANAL = -4 /* los W5_RED_CANALES en uso */, W5_RED_E_NO_DISPONIBLE = -5 };
#define W5_RED_CUERPO_A_TROZOS 0xFFFFFFFFu     /* red_http(…, cuerpo = 0, n = esto): el cuerpo irá con red_enviar() + red_terminar() */
#define W5_RED_CUERPO_MAX 4096                 /* lo más que se pasa ENTERO en red_http; más grande, a trozos */

#define W5_API16(F) \
    F(int, red_conectar, (const unsigned char *addr))    /* arranca: radio + SPP con el puente + saludo. addr = 0: el elegido en Ajustes. W5_OK / W5_ERROR (mirar red_fallo) / W5_NO_DISPONIBLE */ \
    F(void, red_desconectar, (void))                     /* cierra todo (la radio sigue encendida) */ \
    F(int, red_estado, (void))                           /* W5_RED_* */ \
    F(int, red_fallo, (void))                            /* W5_REDF_* */ \
    F(int, red_http, (const char *metodo, const char *url, const char *cabeceras, const void *cuerpo, unsigned n, unsigned banderas)) /* canal o W5_RED_E_*; cabeceras «Nombre: valor\r\n»…; todo se COPIA */ \
    F(int, red_tcp, (const char *host, unsigned puerto, unsigned banderas))   /* canal o W5_RED_E_* */ \
    F(int, red_canal, (int c))                           /* W5_RC_* */ \
    F(int, red_codigo, (int c))                          /* W5_RCOD_* (con W5_RC_CERRADO) */ \
    F(int, red_texto, (int c, char *d, unsigned max))    /* COPIA el porqué del cierre («Name or service not known»…); bytes */ \
    F(int, red_http_estado, (int c))                     /* 200, 404…; 0 = aún no */ \
    F(int, red_cabeceras, (int c, char *d, unsigned max))   /* COPIA las cabeceras de la respuesta (TCP: «ip:puerto»); bytes */ \
    F(int, red_fin_recibido, (int c))                    /* 1 = el otro lado ya no mandará más: lo que quede en la cola es lo último */ \
    F(int, red_disponibles, (int c))                     /* bytes esperando en la cola de recepción */ \
    F(int, red_recibir, (int c, void *d, unsigned max))  /* COPIA y saca; no bloquea (0 = nada ahora) */ \
    F(int, red_enviar, (int c, const void *d, unsigned n))  /* bytes aceptados (hasta 4 KB por llamada; la cola del canal es de 16 KB); el resto, en otra vuelta */ \
    F(int, red_pendiente, (int c))                       /* bytes aún sin salir */ \
    F(int, red_terminar, (int c))                        /* FIN: no mandaré más (fin del cuerpo HTTP a trozos / medio cierre TCP) */ \
    F(void, red_cerrar, (int c))                         /* soltar el canal (si seguía abierto, se aborta). SIEMPRE al acabar */

/* Dónde va el grupo: detrás de la 1.3, del Bluetooth (1.4, 15 funciones) y del sonido (1.5, 2). */
#define W5_RED_CUENTA_(r, n, a) +1
enum { W5_RED_N13 = 0 W5_API(W5_RED_CUENTA_), W5_RED_N14 = 15, W5_RED_N15 = 2, W5_RED_N16 = 0 W5_API16(W5_RED_CUENTA_) };
#define W5_RED_INDICE (W5_RED_N13 + W5_RED_N14 + W5_RED_N15)    /* casilla de red_conectar en la tabla */

typedef struct W5ApiRed {
    W5Api base;
    void (*antes_[W5_RED_N14 + W5_RED_N15])(void);
#define W5_RED_F_(r, nombre, args) r (*nombre) args;
    W5_API16(W5_RED_F_)
#undef W5_RED_F_
} W5ApiRed;
#define W5RED ((const W5ApiRed *)w5)

/* ¿se puede llamar a esto en ESTE sistema? (tabla larga, menor >= 6 y el aparato con red) */
static inline int w5_red_hay(void)
{
    int api;
    if (!w5 || w5->n < (unsigned)(W5_RED_INDICE + W5_RED_N16)) return 0;
    api = ap_consulta(W5_Q_API);
    if (api < 0 || ((api >> 16) & 0xFFFF) != W5_API_MAYOR || (api & 0xFFFF) < W5_RED_MENOR) return 0;
    return (ap_consulta(W5_Q_CAPS) & W5_CAP_RED) != 0;
}

#ifndef W5_RED_SIN_NOMBRES
#define red_conectar (W5RED->red_conectar)
#define red_desconectar (W5RED->red_desconectar)
#define red_estado (W5RED->red_estado)
#define red_fallo (W5RED->red_fallo)
#define red_http (W5RED->red_http)
#define red_tcp (W5RED->red_tcp)
#define red_canal (W5RED->red_canal)
#define red_codigo (W5RED->red_codigo)
#define red_texto (W5RED->red_texto)
#define red_http_estado (W5RED->red_http_estado)
#define red_cabeceras (W5RED->red_cabeceras)
#define red_fin_recibido (W5RED->red_fin_recibido)
#define red_disponibles (W5RED->red_disponibles)
#define red_recibir (W5RED->red_recibir)
#define red_enviar (W5RED->red_enviar)
#define red_pendiente (W5RED->red_pendiente)
#define red_terminar (W5RED->red_terminar)
#define red_cerrar (W5RED->red_cerrar)
#endif
#endif
