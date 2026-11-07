# Apps conectadas a internet: cómo habla tu app con SU servidor

> [!WARNING]
> **senew es un proyecto nuevo; úsalo bajo tu propio riesgo.** La red por Bluetooth es una función joven; no la uses para nada crítico ni para datos que no quieras que pasen por tu computador.

**Guía para quien escribe apps `.sen`.** La referencia de las 18 funciones de red está en [`include/w5_red.h`](../../include/w5_red.h) y en [`sdk-api.md`](sdk-api.md); esta guía explica el modelo y las reglas.

## 0. El modelo: tu app llama a SU backend, SENUS hace de puente

El W580 no tiene WiFi ni TLS, y su plan de datos no se usa. Pero tiene una **radio Bluetooth de fábrica**, y senew la maneja entera. Con eso y con **SENUS** (la aplicación de escritorio de semakers) abierta en tu computador, **cualquier app puede salir a internet**: peticiones HTTP con cualquier verbo (`http://` y `https://`) y sockets TCP (con TLS si se pide). Con el hardware original del teléfono: no se suelda ni se añade nada.

```
 tu app .sen ── API 1.6 (w5_red.h) ──▶ senew ── Bluetooth (SPP) ──▶ SENUS (tu computador) ── DNS · TCP · TLS · HTTP ──▶ internet
               red_http, red_tcp…     4 canales,   radio de fábrica     el «puente»                                     tu servidor
                                      control de flujo   del W580
```

- **No hay un servidor de semakers en medio.** Tu app habla con **tu** servidor (el que tú pongas en la URL); SENUS solo es el módem. No hay cuentas ni claves de semakers que pedir.
- El trabajo pesado (DNS, TLS, certificados, redirecciones, HTTP) lo hace el puente en el computador; el teléfono solo mueve bytes. Por eso una app **no necesita bibliotecas de red ni de criptografía**.
- **Solo hay internet con SENUS abierta** y a alcance de Bluetooth (clase 2: unos 10 m). Sin ella no hay red, y la app tiene que decirlo.
- **Seguridad, dicho claro**: el TLS lo termina el computador, no el teléfono. Entre el teléfono y el computador los datos viajan por el enlace Bluetooth emparejado (con PIN), no cifrados de extremo a extremo por la aplicación. No mandes por ahí lo que no mandarías por Bluetooth.
- **Destinos prohibidos**: por omisión el puente no deja hablar con el propio computador (`localhost`, `127.0.0.0/8`, `::1`, `0.0.0.0`), salvo los puertos que SENUS abre a propósito para sus servicios (música, vídeo, dictado). Ese intento da `W5_RCOD_PROHIBIDO`. La red de casa sí se alcanza, y por supuesto internet.

## 1. Lo que la app NO tiene que hacer (ya está hecho)

**No hace falta optimizar nada para ir rápido.** El sistema y el puente ya hacen:

- el **control de flujo** por canal (si la app no lee, el otro lado espera: no se pierde nada) y los créditos del Bluetooth;
- los **lotes**: el puente junta los datos en tramas grandes y devuelve la ventana por tandas;
- el **enlace**: encender la radio, llamar al puente, reintentar 2 veces si la llamada vence, el saludo;
- **TLS** y la comprobación del certificado, el **DNS**, seguir las **redirecciones** 3xx;
- **4 canales a la vez** sobre un solo enlace;
- **soltar todo** al cerrar la app (canales, red y radio), aunque la app se olvide.

## 2. Las seis reglas

1. **Mirar antes de usar.** `if (w5_red_hay())`: falso en un sistema sin la API 1.6, en un entorno sin radio y en un teléfono sin radio. Si la app debe cargar también ahí, **no** pongas `W5_CAP_RED` en `.caps` y dilo en pantalla.
2. **Todo es asíncrono: nunca esperar en un bucle.** Una orden (`red_conectar`, `red_http`, `red_tcp`) solo **arranca**; lo que pasa se mira en un `ui_cada`. Los datos se mueven en el bucle del SISTEMA, entre una llamada a la app y la siguiente: un `while (!listo) ;` dentro de la app no termina nunca. (En un juego, cada `jg_volcar` mueve también la red; dentro de un cálculo largo, `ap_latido()`.)
3. **En cada vuelta, leer TODO lo que haya**: `while ((n = red_recibir(c, b, sizeof b)) > 0) …`. Leer poco no rompe nada, pero frena: la cola de cada canal es de 16 KB y, llena, el puente espera.
4. **Acabado = `red_canal(c) == W5_RC_CERRADO && red_disponibles(c) == 0`.** Entonces `red_codigo(c)` dice cómo: `W5_RCOD_BIEN` también con un 404 o un 500 (eso es `red_http_estado(c)`); lo demás es un fallo de red con su texto en `red_texto`.
5. **`red_cerrar(c)` SIEMPRE** al acabar con un canal (y para abortar uno abierto): si no, el número no se libera y a la quinta petición da `W5_RED_E_SIN_CANAL`.
6. **`W5_RED_E_OCUPADO` no es un error**: el envío del sistema está lleno un momento; prueba en la vuelta siguiente.

Y una de convivencia: **el Bluetooth es UNO A LA VEZ**. Con la red conectada, conectar a un robot por Bluetooth da error, y al revés `red_conectar` da `W5_RED_FALLO` con `W5_REDF_OCUPADA`.

## 3. Antes de empezar (una vez por teléfono)

En SENUS, apartado **Conexión**: enciende el puente Bluetooth (solo si tu computador tiene Bluetooth) y **empareja** el teléfono; SENUS lo hace por el cable USB, sin búsqueda ni equipo visible, y al desconectar el cable el teléfono queda con internet por ese computador. Para comprobarlo, desde el teléfono abre una app que use la red (por ejemplo `senew-app-red`) y pulsa «Conectar con el puente». Desde ahí cualquier app conecta con `red_conectar(0)` (0 = el puente elegido).

## 4. La plantilla (copiar y cambiar)

[`senew-app-plantilla_red`](https://github.com/semakers/senew-app-plantilla_red): una máquina de estados de cinco fases en un `ui_cada` de 30 ms. Lo esencial:

```c
#include "w5.h"
#include "w5_red.h"
W5_APP(.nombre = "Mi app", .version = 1, .pila = 16384);   /* sin W5_CAP_RED: carga también donde no hay red */

static void cada(W5Ctx c)                                   /* cada 30 ms */
{
    static unsigned char b[2048];
    int k;
    if (s.fase == E_CONECTANDO) {                           /* 1) esperar LISTA (o FALLO) */
        if (red_estado() == W5_RED_LISTA) s.fase = E_PIDIENDO;
        else if (red_estado() == W5_RED_FALLO) s.fase = E_FALLO;            /* red_fallo() dice por qué */
    }
    if (s.fase == E_PIDIENDO) {                             /* 2) abrir el canal */
        s.canal = red_http("GET", URL, 0, 0, 0, W5_RED_PLAZO(15));
        if (s.canal > 0) s.fase = E_LEYENDO;
        else if (s.canal != W5_RED_E_OCUPADO) s.fase = E_FALLO;            /* OCUPADO: en la vuelta siguiente */
    }
    if (s.fase == E_LEYENDO) {
        while ((k = red_recibir(s.canal, b, sizeof b)) > 0) { /* 3) leer TODO */ usar(b, k); }
        if (red_canal(s.canal) == W5_RC_CERRADO && !red_disponibles(s.canal)) {   /* 4) acabó */
            s.fase = red_codigo(s.canal) == W5_RCOD_BIEN ? E_HECHO : E_FALLO;    /* el HTTP: red_http_estado() */
            red_cerrar(s.canal); s.canal = 0;               /* 5) SIEMPRE */
        }
    }
    ui_sucio(c);
}
static void iniciar(W5Ctx c, void *dato) { (void)dato; if (w5_red_hay()) { empezar(); ui_cada(c, 30, cada); } }
/* empezar(): si red_estado() ya es LISTA, a pedir; si no, red_conectar(0) y a esperar */
```

Lo que se muestra en pantalla se **copia** a la memoria de la app (`ui_fmt` dura un cuadro). Compila con `make` (ver [`sdk.md`](sdk.md)) y súbela con SENUS → Apps → «Subir .sen».

## 5. Recetas

### 5.1 Otros verbos, con cabeceras y cuerpo (POST, PUT, PATCH, DELETE…)

```c
static const char json[] = "{\"nombre\": \"W580\"}";
c = red_http("POST", "https://api.ejemplo.com/cosas",
             "Content-Type: application/json\r\nAuthorization: Bearer XXXX\r\n",   /* «Nombre: valor\r\n» cada una */
             json, sizeof json - 1, W5_RED_PLAZO(20));
```

Todo se **copia** en la llamada (la app puede reutilizar sus búferes enseguida). El cuerpo entero llega hasta `W5_RED_CUERPO_MAX` (4 KB). El `Content-Length` lo pone el puente. Las cabeceras de la respuesta: `red_cabeceras(c, d, max)`. Para no seguir una redirección: `W5_RED_SIN_REDIR`. Un servidor con certificado propio: `W5_RED_SIN_VERIFICAR` (solo para pruebas; no lo dejes en una app publicada).

### 5.2 Un cuerpo grande, a trozos (subir un fichero)

```c
c = red_http("PUT", url, cab, 0, W5_RED_CUERPO_A_TROZOS, 0);
/* … y en cada vuelta, mientras quede: */
k = red_enviar(c, datos + hecho, total - hecho);        /* acepta hasta 4 KB por llamada; el resto, otra vuelta */
if (k > 0) hecho += k;
if (hecho == total && !terminado) { red_terminar(c); terminado = 1; }   /* fin del cuerpo */
/* la respuesta se lee igual que siempre */
```

### 5.3 Bajar algo y guardarlo

El cuerpo llega a trozos: se junta en memoria de la app (`ap_mem_pedir`) y se guarda **entero** al acabar con `ap_fich_escribir` (atómico; pide `W5_CAP_FICHEROS`). `red_cabeceras` trae el `Content-Length` (si el servidor lo manda) para reservar de una vez. **No guardar en la flash mientras suena música**: escribir en la NOR frena al DSP.

### 5.4 Un socket TCP (un protocolo propio, un chat, MQTT o WebSocket hechos a mano)

```c
c = red_tcp("servidor.ejemplo.com", 443, W5_RED_TLS | W5_RED_PLAZO(10));   /* sin W5_RED_TLS: TCP a pelo */
/* cuando red_canal(c) == W5_RC_ABIERTO: */
red_enviar(c, linea, n);                                  /* lo que no quepa, en otra vuelta (red_pendiente: lo que falta por salir) */
/* y leer como siempre. El servidor cerró: red_fin_recibido(c). Medio cierre por nuestra parte: red_terminar(c). */
```

`red_cabeceras` de un socket da «ip:puerto» del otro lado.

### 5.5 Varias cosas a la vez

Hasta **4 canales** (`W5_RED_CANALES`): una descarga y una consulta, o un socket abierto y peticiones sueltas. Cada uno con su cola y su ventana: uno lento no frena a los demás. A la quinta, `W5_RED_E_SIN_CANAL` hasta que se cierre uno.

### 5.6 Un flujo continuo (radio por internet, una lista de canciones…)

El enlace da del orden de 100–137 KB/s y una canción de 128 kbps pide 16 KB/s: hay margen de sobra. Se lee en cada vuelta a un colchón circular y se consume de él; el control de flujo hace el resto (con el colchón lleno, la app deja de leer y el servidor espera). **Para MÚSICA no hace falta hacerlo a mano** (API 1.8, `w5_musica_red.h`): la app pone las URLs de los MP3 en la cola del SISTEMA (`mu_red_poner`, `mu_red_tocar`) y él las baja, las toca y sigue con la app cerrada; ejemplo: [`senew-app-musica`](https://github.com/semakers/senew-app-musica).

## 6. Cifras

Medidas en un W580 con la primera implementación del puente (un servicio en Linux). SENUS implementa el mismo protocolo y apunta a las mismas marcas; no las doy por garantizadas hasta que se midan con ella.

| qué | cifra |
|---|---|
| bajar sin pausas (socket) | ~137 KB/s |
| subir sin pausas (socket) | ~154 KB/s |
| los dos sentidos a la vez | ~88 + 88 KB/s |
| una app que atiende cada 33 ms (30 cuadros/s): bajar / subir / eco | ~103 / 78 / 65 KB/s |
| ida y vuelta de 32 B | ~28 ms (la latencia no baja de ~25 ms) |
| 1 MB por HTTPS desde una app | ~7 s |
| SPP a pelo, sin la capa de red (el techo del enlace) | ~160 ↑ / 148 ↓ KB/s |

## 7. Qué hacer con cada fallo

**`red_fallo()`** (con `red_estado() == W5_RED_FALLO`):

| valor | qué pasa | qué hacer |
|---|---|---|
| `W5_REDF_SIN_RADIO` | este aparato no tiene radio | decirlo; no reintentar |
| `W5_REDF_SIN_PUENTE` | no hay puente elegido | emparejar desde SENUS → Conexión |
| `W5_REDF_RADIO` | la radio no arrancó | reintentar una vez; si sigue, reiniciar el teléfono |
| `W5_REDF_ENLACE` | el puente no contesta (SENUS cerrada o lejos, Bluetooth del computador apagado) | ya se reintentó 2 veces: decirlo («¿SENUS está abierta y a menos de 10 m?») y dejar reintentar a mano |
| `W5_REDF_SALUDO` | contestó algo que no es un puente | el aparato elegido no es SENUS |
| `W5_REDF_CAIDA` | se cortó estando conectada (los canales quedan CERRADOS) | `red_conectar(0)` otra vez y repetir lo que faltara |
| `W5_REDF_OCUPADA` | un robot usa el Bluetooth | esperar a que se desconecte |

**`red_codigo(c)`** (con el canal CERRADO): `BIEN` (también 404/500: mira `red_http_estado`), `DNS` (el nombre no existe), `RECHAZADA` (puerto cerrado), `PLAZO` (no contesta: sube `W5_RED_PLAZO`), `TLS` (certificado malo o caducado), `HTTP` (respuesta ilegible), `RED`, `CORTADA` (la otra punta cortó), `PROHIBIDO` (destino no permitido, §0), `DEMASIADOS`, `PROTOCOLO`, `ABORTADA` (la cerró la app), `INTERNO`, `SIN_SESION` (la red cayó). El porqué en palabras: `red_texto(c, …)`.

## 8. Límites (dichos claro)

- **Hace falta SENUS abierta** en un computador con Bluetooth, a alcance. Sin él no hay red: la app tiene que decirlo.
- HTTP/1.1. **Sin HTTP/2 ni WebSocket ya hechos**: sí con un socket (`red_tcp` + TLS) y el protocolo escrito en la app.
- Un enlace, 4 canales; uno a la vez con los robots por Bluetooth.
- La latencia no baja de ~25 ms.
- Cuerpo entero hasta 4 KB por llamada; más grande, a trozos (§5.2). Las respuestas no tienen tope (llegan a trozos).

## 9. Cómo se prueba

- **En el emulador y en cualquier entorno sin radio** `w5_red_hay()` da 0 y la app debe enseñarlo (la plantilla lo hace). Lo que no es red se prueba como cualquier app.
- **En el teléfono**: sube la `.sen` con SENUS y ábrela. Ejemplos que ya funcionan: [`senew-app-red`](https://github.com/semakers/senew-app-red) (GET, 1 MB, POST JSON, TCP+TLS a mano) y [`senew-app-plantilla_red`](https://github.com/semakers/senew-app-plantilla_red).
- Para que tu servidor sea de verdad **tuyo**: ponlo detrás de HTTPS con un certificado válido; el puente comprueba el certificado.
