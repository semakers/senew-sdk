# Internet-connected apps: how your app talks to ITS server

> [!WARNING]
> **senew is a new project; use it at your own risk.** Networking over Bluetooth is a young feature; do not rely on it for anything critical or for data you do not want to pass through your computer.

**A guide for people writing `.sen` apps.** The reference for the 18 network functions is in [`include/w5_red.h`](../../include/w5_red.h) and in [`sdk-api.md`](sdk-api.md); this guide explains the model and the rules.

## 0. The model: your app calls ITS backend, SENUS is the bridge

The W580 has no WiFi and no TLS, and its mobile data plan is not used. But it has a **factory Bluetooth radio**, and senew drives all of it. With that and **SENUS** (semakers' desktop application) open on your computer, **any app can reach the internet**: HTTP requests with any verb (`http://` and `https://`) and TCP sockets (with TLS on request). With the phone's original hardware: nothing to solder or add.

```
 your .sen app ── API 1.6 (w5_red.h) ──▶ senew ── Bluetooth (SPP) ──▶ SENUS (your computer) ── DNS · TCP · TLS · HTTP ──▶ internet
                red_http, red_tcp…      4 channels,  the W580's own radio   the "bridge"                                    your server
                                        flow control
```

- **There is no semakers server in the middle.** Your app talks to **your** server (whatever URL you put in); SENUS is only the modem. There are no semakers accounts or keys to ask for.
- The heavy work (DNS, TLS, certificates, redirects, HTTP) is done by the bridge on the computer; the phone only moves bytes. That is why an app **needs no networking or crypto libraries**.
- **There is internet only while SENUS is open** and within Bluetooth range (class 2: about 10 m). Without it there is no network, and the app has to say so.
- **Security, plainly**: TLS is terminated on the computer, not on the phone. Between the phone and the computer the data travels over the paired Bluetooth link (PIN-paired), not end-to-end encrypted by the application. Do not send over it what you would not send over Bluetooth.
- **Forbidden destinations**: by default the bridge does not let the phone talk to the computer itself (`localhost`, `127.0.0.0/8`, `::1`, `0.0.0.0`), except for the ports SENUS deliberately opens for its own services (music, video, dictation). Trying gives `W5_RCOD_PROHIBIDO`. Your home network is reachable, and of course the internet.

## 1. What the app does NOT have to do (it is done already)

**You do not need to optimise anything to go fast.** The system and the bridge already do:

- per-channel **flow control** (if the app does not read, the other side waits: nothing is lost) and Bluetooth credits;
- **batching**: the bridge groups data in large frames and returns the window in steps;
- the **link**: switching the radio on, calling the bridge, retrying twice if the call times out, the handshake;
- **TLS** and certificate checking, **DNS**, following **3xx redirects**;
- **4 channels at once** over a single link;
- **releasing everything** when the app closes (channels, network and radio), even if the app forgets.

## 2. The six rules

1. **Look before you use.** `if (w5_red_hay())`: false on a system without API 1.6, in an environment without a radio and on a phone without a radio. If the app must load there too, do **not** put `W5_CAP_RED` in `.caps` and say so on screen.
2. **Everything is asynchronous: never wait in a loop.** A command (`red_conectar`, `red_http`, `red_tcp`) only **starts**; you check what happened in a `ui_cada`. Data moves in the SYSTEM's loop, between one call into the app and the next: a `while (!ready) ;` inside the app never ends. (In a game, every `jg_volcar` also moves the network; during a long computation, `ap_latido()`.)
3. **On every turn, read EVERYTHING there is**: `while ((n = red_recibir(c, b, sizeof b)) > 0) …`. Reading little breaks nothing but slows things down: each channel's queue is 16 KB and, when full, the bridge waits.
4. **Finished = `red_canal(c) == W5_RC_CERRADO && red_disponibles(c) == 0`.** Then `red_codigo(c)` tells you how: `W5_RCOD_BIEN` even for a 404 or a 500 (that is `red_http_estado(c)`); anything else is a network failure with its text in `red_texto`.
5. **ALWAYS `red_cerrar(c)`** when you are done with a channel (and to abort an open one): otherwise the number is not released and the fifth request returns `W5_RED_E_SIN_CANAL`.
6. **`W5_RED_E_OCUPADO` is not an error**: the system's send queue is full for a moment; try again next turn.

And one rule of coexistence: **Bluetooth is ONE AT A TIME**. With the network connected, connecting to a Bluetooth robot fails, and vice versa `red_conectar` returns `W5_RED_FALLO` with `W5_REDF_OCUPADA`.

## 3. Before you start (once per phone)

In SENUS, the **Conexión** (Connection) section: switch the Bluetooth bridge on (only if your computer has Bluetooth) and **pair** the phone; SENUS does it over the USB cable, with no scanning or visible device, and once you unplug the cable the phone has internet through that computer. To check, open an app that uses the network on the phone (e.g. `senew-app-red`) and press "Conectar con el puente" (connect to the bridge). From then on any app connects with `red_conectar(0)` (0 = the chosen bridge).

## 4. The template (copy and change)

[`senew-app-plantilla_red`](https://github.com/semakers/senew-app-plantilla_red): a five-phase state machine inside a 30 ms `ui_cada`. The essentials:

```c
#include "w5.h"
#include "w5_red.h"
W5_APP(.nombre = "Mi app", .version = 1, .pila = 16384);   /* no W5_CAP_RED: also loads where there is no network */

static void cada(W5Ctx c)                                   /* every 30 ms */
{
    static unsigned char b[2048];
    int k;
    if (s.fase == E_CONECTANDO) {                           /* 1) wait for READY (or FAILURE) */
        if (red_estado() == W5_RED_LISTA) s.fase = E_PIDIENDO;
        else if (red_estado() == W5_RED_FALLO) s.fase = E_FALLO;            /* red_fallo() says why */
    }
    if (s.fase == E_PIDIENDO) {                             /* 2) open the channel */
        s.canal = red_http("GET", URL, 0, 0, 0, W5_RED_PLAZO(15));
        if (s.canal > 0) s.fase = E_LEYENDO;
        else if (s.canal != W5_RED_E_OCUPADO) s.fase = E_FALLO;            /* BUSY: next turn */
    }
    if (s.fase == E_LEYENDO) {
        while ((k = red_recibir(s.canal, b, sizeof b)) > 0) { /* 3) read EVERYTHING */ usar(b, k); }
        if (red_canal(s.canal) == W5_RC_CERRADO && !red_disponibles(s.canal)) {   /* 4) finished */
            s.fase = red_codigo(s.canal) == W5_RCOD_BIEN ? E_HECHO : E_FALLO;    /* the HTTP status: red_http_estado() */
            red_cerrar(s.canal); s.canal = 0;               /* 5) ALWAYS */
        }
    }
    ui_sucio(c);
}
static void iniciar(W5Ctx c, void *dato) { (void)dato; if (w5_red_hay()) { empezar(); ui_cada(c, 30, cada); } }
/* empezar(): if red_estado() is already READY, request; otherwise red_conectar(0) and wait */
```

(*estado* = state, *fase* = phase, *canal* = channel, *leyendo* = reading, *hecho* = done, *fallo* = failure, *pidiendo* = requesting, *conectando* = connecting, *empezar* = start, *iniciar* = init.)

What you show on screen must be **copied** into the app's memory (`ui_fmt` lasts one frame). Build with `make` (see [`sdk.md`](sdk.md)) and upload with SENUS → Apps → "Subir .sen".

## 5. Recipes

### 5.1 Other verbs, with headers and body (POST, PUT, PATCH, DELETE…)

```c
static const char json[] = "{\"nombre\": \"W580\"}";
c = red_http("POST", "https://api.example.com/things",
             "Content-Type: application/json\r\nAuthorization: Bearer XXXX\r\n",   /* "Name: value\r\n" each */
             json, sizeof json - 1, W5_RED_PLAZO(20));
```

Everything is **copied** during the call (the app can reuse its buffers right away). A whole body can be up to `W5_RED_CUERPO_MAX` (4 KB). The bridge adds `Content-Length`. Response headers: `red_cabeceras(c, d, max)`. To not follow a redirect: `W5_RED_SIN_REDIR`. A server with a self-signed certificate: `W5_RED_SIN_VERIFICAR` (for tests only; do not leave it in a published app).

### 5.2 A large body, in chunks (uploading a file)

```c
c = red_http("PUT", url, cab, 0, W5_RED_CUERPO_A_TROZOS, 0);
/* … and on every turn, while there is data left: */
k = red_enviar(c, data + done, total - done);           /* accepts up to 4 KB per call; the rest, next turn */
if (k > 0) done += k;
if (done == total && !finished) { red_terminar(c); finished = 1; }   /* end of body */
/* the response is read as usual */
```

### 5.3 Download something and save it

The body arrives in chunks: collect it in the app's memory (`ap_mem_pedir`) and save it **whole** at the end with `ap_fich_escribir` (atomic; needs `W5_CAP_FICHEROS`). `red_cabeceras` returns the `Content-Length` (if the server sends one) so you can reserve it all at once. **Do not write to flash while music is playing**: writing to the NOR slows the DSP down.

### 5.4 A TCP socket (your own protocol, a chat, hand-made MQTT or WebSocket)

```c
c = red_tcp("server.example.com", 443, W5_RED_TLS | W5_RED_PLAZO(10));   /* without W5_RED_TLS: plain TCP */
/* when red_canal(c) == W5_RC_ABIERTO: */
red_enviar(c, line, n);                                   /* whatever does not fit goes next turn (red_pendiente: what is still to go out) */
/* and read as usual. Server closed: red_fin_recibido(c). Half-close from our side: red_terminar(c). */
```

`red_cabeceras` on a socket returns the other side's "ip:port".

### 5.5 Several things at once

Up to **4 channels** (`W5_RED_CANALES`): a download and a query, or an open socket and one-off requests. Each has its own queue and window: a slow one does not hold up the others. On the fifth, `W5_RED_E_SIN_CANAL` until one is closed.

### 5.6 A continuous stream (internet radio, a playlist…)

The link gives on the order of 100–137 KB/s and a 128 kbps song needs 16 KB/s: plenty of margin. Read on every turn into a circular buffer and consume from it; flow control does the rest (with the buffer full the app stops reading and the server waits). **For MUSIC you do not have to do it by hand** (API 1.8, `w5_musica_red.h`): the app puts the MP3 URLs in the SYSTEM's queue (`mu_red_poner`, `mu_red_tocar`) and the system downloads and plays them, and keeps playing with the app closed; example: [`senew-app-musica`](https://github.com/semakers/senew-app-musica).

## 6. Numbers

Measured on a W580 with the first implementation of the bridge (a Linux service). SENUS implements the same protocol and aims for the same marks; do not take them as guaranteed until they are measured with it.

| what | figure |
|---|---|
| download without pauses (socket) | ~137 KB/s |
| upload without pauses (socket) | ~154 KB/s |
| both directions at once | ~88 + 88 KB/s |
| an app serving every 33 ms (30 frames/s): download / upload / echo | ~103 / 78 / 65 KB/s |
| 32 B round trip | ~28 ms (latency does not go below ~25 ms) |
| 1 MB over HTTPS from an app | ~7 s |
| raw SPP, without the network layer (the link's ceiling) | ~160 ↑ / 148 ↓ KB/s |

## 7. What to do with each failure

**`red_fallo()`** (when `red_estado() == W5_RED_FALLO`):

| value | what happened | what to do |
|---|---|---|
| `W5_REDF_SIN_RADIO` | this device has no radio | say so; do not retry |
| `W5_REDF_SIN_PUENTE` | no bridge chosen | pair from SENUS → Conexión |
| `W5_REDF_RADIO` | the radio did not start | retry once; if it persists, restart the phone |
| `W5_REDF_ENLACE` | the bridge does not answer (SENUS closed or too far, the computer's Bluetooth off) | already retried twice: say so ("is SENUS open and within 10 m?") and let the user retry |
| `W5_REDF_SALUDO` | it answered with something that is not a bridge | the chosen device is not SENUS |
| `W5_REDF_CAIDA` | the link dropped while connected (channels are left CLOSED) | `red_conectar(0)` again and redo whatever was missing |
| `W5_REDF_OCUPADA` | a robot is using Bluetooth | wait until it disconnects |

**`red_codigo(c)`** (with the channel CLOSED): `BIEN` (ok; also for 404/500: check `red_http_estado`), `DNS` (the name does not exist), `RECHAZADA` (port closed), `PLAZO` (no answer: raise `W5_RED_PLAZO`), `TLS` (bad or expired certificate), `HTTP` (unreadable response), `RED`, `CORTADA` (the other end cut it), `PROHIBIDO` (destination not allowed, §0), `DEMASIADOS`, `PROTOCOLO`, `ABORTADA` (the app closed it), `INTERNO`, `SIN_SESION` (the network dropped). The reason in words: `red_texto(c, …)`.

## 8. Limits (stated plainly)

- **SENUS must be open** on a computer with Bluetooth, within range. Without it there is no network: the app has to say so.
- HTTP/1.1. **No ready-made HTTP/2 or WebSocket**: yes with a socket (`red_tcp` + TLS) and the protocol written in the app.
- One link, 4 channels; one at a time with Bluetooth robots.
- Latency does not go below ~25 ms.
- A whole body up to 4 KB per call; larger goes in chunks (§5.2). Responses have no cap (they arrive in chunks).

## 9. How to test

- **In the emulator and in any environment without a radio** `w5_red_hay()` returns 0 and the app must show it (the template does). Whatever is not network is tested like any app.
- **On the phone**: upload the `.sen` with SENUS and open it. Examples that already work: [`senew-app-red`](https://github.com/semakers/senew-app-red) (GET, 1 MB, JSON POST, hand-made TCP+TLS) and [`senew-app-plantilla_red`](https://github.com/semakers/senew-app-plantilla_red).
- To make your server really **yours**: put it behind HTTPS with a valid certificate; the bridge checks the certificate.
