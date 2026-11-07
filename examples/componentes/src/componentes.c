/* componentes.c — (MD, API 1.3) el muestrario del sistema de diseño: una app de fuera que usa los componentes SEMÁNTICOS
 * (fila con accesorio, botones por prioridad, chips, tarjeta, deslizador, aviso breve) y los tokens, sin saber qué diseño hay.
 * Sel. izq. pide el OTRO diseño para ella (ui_diseno_pedir): al salir, el sistema vuelve al suyo. Ejemplo del SDK. */
#include "w5.h"

W5_APP(.nombre = "Componentes", .version = 1, .icono_gen = W5_IC_AJUSTES);

static int wifi = 1, avisos, modo = 1, brillo = 6, chip = 1;
static char diseno[24];

static W5W fila_ctl(W5Ctx c, int enf, int t, void *dato)
{
    static const char *const tit[3] = { "Conexión", "Avisar al terminar", "Modo" };
    static const char *const sub[3] = { "interruptor", "casilla", "opción" };
    int i = (int)(W5Pal)dato;
    W5FilaDatos f = { .titulo = tit[i], .subtitulo = sub[i], .accesorio = i == 0 ? W5_ACC_INTERRUPTOR : i == 1 ? W5_ACC_CASILLA : W5_ACC_OPCION,
                      .activo = (unsigned char)(i == 0 ? wifi : i == 1 ? avisos : modo) };
    (void)c; (void)t;
    return Caja(A_AL(30), A_HIJO(ui_fila_de(&f, enf)));
}
static void activar_ctl(W5Ctx c, void *dato)
{
    int i = (int)(W5Pal)dato;
    if (i == 0) wifi = !wifi; else if (i == 1) avisos = !avisos; else modo = !modo;
    ui_sucio(ui_mi_pantalla(c));
}
static W5W boton(W5Ctx c, int enf, int t, void *dato)
{
    static const char *const txt[3] = { "Guardar", "Probar", "Más" };
    (void)c; (void)t;
    return ui_boton(txt[(int)(W5Pal)dato], (int)(W5Pal)dato, enf);
}
static void pulsar(W5Ctx c, void *dato) { (void)c; ui_aviso_breve((int)(W5Pal)dato == 0 ? "Guardado." : "Botón pulsado."); }
static W5W un_chip(W5Ctx c, int enf, int t, void *dato)
{
    static const char *const txt[3] = { "Hoy", "Semana", "Mes" };
    (void)c; (void)t;
    return ui_chip(txt[(int)(W5Pal)dato], chip == (int)(W5Pal)dato, enf);
}
static void elegir_chip(W5Ctx c, void *dato) { chip = (int)(W5Pal)dato; ui_sucio(ui_mi_pantalla(c)); }
static W5W desliz(W5Ctx c, int enf, int t, void *dato)
{
    (void)c; (void)t; (void)dato;
    return RellenoXY(8, 2, Columna(A_CRUZADO(W5P_ESTIRAR), HIJOS(
        Texto(ui_fmt("Brillo: %d de 10", brillo), A_MARCAS(W5_F_ESTILO(W5_T_ETIQUETA)), A_COLOR(ui_token(W5_TK_SOBRE_SUPERFICIE_VARIANTE))),
        ui_deslizador(brillo, 10, enf))));
}
static int tecla(W5Ctx c, int k, void *dato)
{
    (void)dato;
    if (k == W5K_IZQ || k == W5K_DER) { brillo += k == W5K_DER ? 1 : -1; if (brillo < 0) brillo = 0; if (brillo > 10) brillo = 10; ui_sucio(ui_mi_pantalla(c)); return 1; }
    if (k == W5K_SEL_IZQ) {                                   /* el otro diseño, solo para esta app */
        ui_diseno(diseno, sizeof diseno);
        ui_diseno_pedir(diseno[0] == 'm' ? "clasico" : "material3");
        ui_sucio(ui_mi_pantalla(c));
        return 1;
    }
    return ui_teclas_defecto(c, k);
}
static W5W construir(W5Ctx c, void *dato)
{
    (void)c; (void)dato;
    ui_diseno(diseno, sizeof diseno);
    return Teclas(tecla, ui_andamio("Componentes",
        Lista(HIJOS(
            RellenoXY(8, 4, Texto(ui_fmt("Diseño: %s", diseno), A_MARCAS(W5_F_ESTILO(W5_T_TITULO)), A_COLOR(ui_token(W5_TK_PRIMARIO)))),
            Enfocable(fila_ctl, A_DATO(0), A_AL_ACTIVAR(activar_ctl), A_AUTOFOCO(1)),
            Enfocable(fila_ctl, A_DATO(1), A_AL_ACTIVAR(activar_ctl)),
            Enfocable(fila_ctl, A_DATO(2), A_AL_ACTIVAR(activar_ctl)),
            Enfocable(desliz),
            RellenoXY(8, 4, Fila(A_ESPACIO(6), HIJOS(
                Enfocable(un_chip, A_DATO(0), A_AL_ACTIVAR(elegir_chip)),
                Enfocable(un_chip, A_DATO(1), A_AL_ACTIVAR(elegir_chip)),
                Enfocable(un_chip, A_DATO(2), A_AL_ACTIVAR(elegir_chip))))),
            RellenoXY(8, 4, ui_tarjeta(Columna(A_ESPACIO(4), HIJOS(
                Texto("Tarjeta", A_MARCAS(W5_F_ESTILO(W5_T_TITULO))),
                Parrafo("Un bloque agrupado. El texto usa los estilos del diseño en uso.", A_COLOR(ui_token(W5_TK_SOBRE_SUPERFICIE_VARIANTE))))), 0)),
            RellenoXY(8, 4, Fila(A_ESPACIO(6), HIJOS(
                Enfocable(boton, A_DATO(0), A_AL_ACTIVAR(pulsar)),
                Enfocable(boton, A_DATO(1), A_AL_ACTIVAR(pulsar)),
                Enfocable(boton, A_DATO(2), A_AL_ACTIVAR(pulsar))))))),
        "Diseño", "Atrás"));
}
W5Pal app_suceso(int suceso, W5Pal a, W5Pal b)
{
    (void)a; (void)b;
    if (suceso == W5_ABRIR) return (W5Pal)ui_pantalla(construir, NULL, NULL, NULL);
    return 0;
}
