/* w5_camara.h — LA CÁMARA PARA APPS (.sen): la ampliación 1.9 de la tabla `w5` (docs/es/sdk-api.md). El sistema la repite en su propia cabecera interna (el árbol del sistema comprueba que las dos dicen lo mismo).
 *
 * LO QUE HAY: el visor de la cámara del W580 (sensor OV2640): cuadros de 176×144 a ~5-7 por segundo. El SISTEMA la enciende, vigila el
 * flujo desde su tic (sin esperas dentro de la app) y, cuando llega un cuadro NUEVO y entero, la app lo toma con cam_cuadro:
 *   - W5_CAM_RGB565: listo para li_pixeles (1.8): RGB565 con el byte ALTO primero, 176×144 (o 144×176 si se gira 90° o 270°);
 *   - W5_CAM_YUYV:   el cuadro tal cual sale del sensor (Y1 U Y2 V, 352 B por fila, sin girar): para codificar una foto (JPEG) sin perder color.
 * Los primeros ~1,2 s tras cam_abrir no se entregan cuadros (la exposición automática aún converge: salen negros): cam_info dice ASENTANDO.
 * cam_abrir TARDA ~0,75 s (programa el sensor) y se llama UNA vez, al abrir la pantalla. Si la app se cierra sin cam_cerrar, la apaga el sistema.
 *
 *     W5_APP(.nombre = "Mi cámara", .api_menor = W5_CAM_MENOR, .caps = W5_CAP_CAMARA, …);   // sin W5_CAP_CAMARA en .caps: opcional (w5_camara_hay)
 *     static unsigned short pix[W5_CAM_ANCHO * W5_CAM_ALTO];
 *     iniciar:  if (w5_camara_hay() && cam_abrir() == W5_OK) ui_cada(c, 40, cada);
 *     cada:     if (cam_cuadro(pix, W5_CAM_RGB565) == 1) ui_sucio(c);            // y en la Pintura: li_pixeles(l, x, y, W5_CAM_ANCHO, W5_CAM_ALTO, pix)
 *     desechar: cam_cerrar();
 *
 * LAS FOTOS: la app las codifica (JPEG; p. ej. senew-app-camara/camara/jpeg_cod.c desde el YUYV) y el SISTEMA las guarda en la tarjeta, en la carpeta de fotos del
 * original (W5_CAM_CARPETA), con su numeración: cam_foto_guardar elige el nombre DSCnnnnn.JPG siguiente y lo devuelve. La app no puede escribir en otro
 * sitio de la tarjeta, y cam_foto_borrar solo borra fotos de esa carpeta. Para verlas: fs_listar(W5_CAM_CARPETA, …) e im_abrir (1.2, W5_CAP_IMAGENES).
 * No hay flash ni LED (no se sabe cómo se encienden), ni la foto grande del sensor (2 MP en JPEG: está en el backlog).
 * Ejemplo: senew-app-camara (la app «Cámara»). */
#ifndef W5_CAMARA_H
#define W5_CAMARA_H
#include "w5.h"

#define W5_CAM_MENOR 9                         /* la menor de la API que trae este grupo */
#define W5_CAM_ANCHO 176                       /* el cuadro sin girar */
#define W5_CAM_ALTO 144
#define W5_CAM_BYTES (W5_CAM_ANCHO * W5_CAM_ALTO * 2)   /* lo que ocupa un cuadro en cualquiera de los dos formatos (50 688 B) */
#define W5_CAM_CARPETA "/tarjeta/DCIM/100MSDCF"   /* donde guarda las fotos cam_foto_guardar (la del original: DSC01404.JPG, MOV01405.3GP…); se lista con fs_listar */
#define W5_CAM_FOTO_MAX (256 * 1024)            /* lo más que mide una foto que se guarda */
enum { W5_CAP_CAMARA = 1 << 26 };              /* hay cámara (el W580; en el banco de PC, una de mentira con barras de colores) */
/* cam_cuadro(dst, formato): el formato y, para RGB565, el giro (en sentido horario; 1 y 3 dan 144 de ancho × 176 de alto) */
enum { W5_CAM_RGB565 = 0, W5_CAM_YUYV = 1 };
#define W5_CAM_GIRO(g) (((g) & 3) << 4)
/* W5CamInfo.estado */
enum { W5_CAM_CERRADA = 0, W5_CAM_ASENTANDO, W5_CAM_EN_VIVO, W5_CAM_FALLO };
typedef struct {
    unsigned char estado, mentira, rsv[2];     /* W5_CAM_*; 1 = la cámara de mentira del banco de PC */
    short ancho, alto;                         /* del cuadro sin girar: 176 × 144 */
    int error;                                 /* el último fallo del sensor al abrir (< 0; 0 = ninguno) */
    unsigned cuadros, entregados;              /* cuadros enteros que han llegado desde cam_abrir, y los que la app tomó */
    unsigned ms_abrir, ms_ultimo;              /* lo que tardó cam_abrir; ap_ms() del último cuadro entregado */
} W5CamInfo;

/* LAS MISMAS LÍNEAS que W5_API19 en la cabecera interna del sistema */
#define W5_API19(F) \
    F(int, cam_abrir, (void))                            /* enciende y programa el sensor (~0,75 s). W5_OK (también si ya estaba) / W5_NO_DISPONIBLE / W5_ERROR (cam_info.error) */ \
    F(int, cam_cerrar, (void))                           /* la apaga. W5_OK */ \
    F(int, cam_cuadro, (void *dst, int formato))         /* 1 = un cuadro NUEVO copiado a dst (W5_CAM_BYTES); 0 = aún no hay; W5_ERROR / W5_NO_DISPONIBLE */ \
    F(int, cam_info, (W5CamInfo *i))                     /* la COPIA a i. W5_OK / W5_ERROR */ \
    F(int, cam_foto_guardar, (const void *jpeg, unsigned n, char *nombre))   /* un JPEG (FF D8 … FF D9) a W5_CAM_CARPETA como DSCnnnnn.JPG, con el número SIGUIENTE (como el original); su nombre a nombre[16]. W5_OK / W5_NO_DISPONIBLE (sin tarjeta) / W5_ERROR */ \
    F(int, cam_foto_borrar, (const char *nombre))        /* borra W5_CAM_CARPETA/nombre; SOLO un «DSCnnnnn.JPG». W5_OK / W5_ERROR */

/* Dónde va el grupo: detrás de la 1.3, del Bluetooth (1.4, 15), del sonido (1.5, 2), de la red (1.6, 18), de la pantalla (1.7, 4) y de la música por la red (1.8, 6). */
#define W5_CAM_CUENTA_(r, n, a) +1
enum { W5_CAM_N13 = 0 W5_API(W5_CAM_CUENTA_), W5_CAM_N14 = 15, W5_CAM_N15 = 2, W5_CAM_N16 = 18, W5_CAM_N17 = 4, W5_CAM_N18 = 6,
       W5_CAM_N19 = 0 W5_API19(W5_CAM_CUENTA_) };
#define W5_CAM_INDICE (W5_CAM_N13 + W5_CAM_N14 + W5_CAM_N15 + W5_CAM_N16 + W5_CAM_N17 + W5_CAM_N18)   /* casilla de cam_abrir en la tabla */

typedef struct W5ApiCam {
    W5Api base;
    void (*antes_[W5_CAM_N14 + W5_CAM_N15 + W5_CAM_N16 + W5_CAM_N17 + W5_CAM_N18])(void);
#define W5_CAM_F_(r, nombre, args) r (*nombre) args;
    W5_API19(W5_CAM_F_)
#undef W5_CAM_F_
} W5ApiCam;
#define W5CAM ((const W5ApiCam *)w5)

/* ¿están las funciones en ESTE sistema (tabla larga y menor >= 9) y hay cámara? */
static inline int w5_camara_hay(void)
{
    int api;
    if (!w5 || w5->n < (unsigned)(W5_CAM_INDICE + W5_CAM_N19)) return 0;
    api = ap_consulta(W5_Q_API);
    if (api < 0 || ((api >> 16) & 0xFFFF) != W5_API_MAYOR || (api & 0xFFFF) < W5_CAM_MENOR) return 0;
    return (ap_consulta(W5_Q_CAPS) & W5_CAP_CAMARA) != 0;
}

#ifndef W5_CAM_SIN_NOMBRES
#define cam_abrir (W5CAM->cam_abrir)
#define cam_cerrar (W5CAM->cam_cerrar)
#define cam_cuadro (W5CAM->cam_cuadro)
#define cam_info (W5CAM->cam_info)
#define cam_foto_guardar (W5CAM->cam_foto_guardar)
#define cam_foto_borrar (W5CAM->cam_foto_borrar)
#endif
#endif
