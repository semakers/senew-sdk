# SDK guide: your first `.sen` app, from scratch

> [!WARNING]
> **senew is a new project; use it at your own risk.** The system, the SDK and the tools are young software for a discontinued phone. Back up your phone before installing anything and try things in the emulator first.

A senew app is a **`.sen`** file: a small ARM binary that the system discovers in memory or on the card, shows in the menu with its icon, loads, runs and unloads. The app does not link against the system: all it sees is a **function table** (the API) that the system hands it when it opens.
This SDK contains everything needed to write, build and package an app **outside** the system tree: headers, linker script, start-up code, the packager, a reusable `Makefile` and a template.

See also: [`sdk-api.md`](sdk-api.md) (the API, function by function) · [`apps-con-internet.md`](apps-con-internet.md) (apps that talk to their own server) · real example apps at [github.com/semakers](https://github.com/semakers) (`senew-app-*`).

## 1. What you need

| Tool | What for | Tested versions |
|---|---|---|
| `arm-none-eabi-gcc` (bare-metal toolchain) | build the app | Arm GNU Toolchain **15.2.Rel1** (macOS arm64) and **15.2.0** (Fedora, package `arm-none-eabi-gcc-cs`) |
| GNU make | `Makefile` | 4.x (on macOS use Homebrew's: `brew install make`, then `gmake` or put `gnubin` in your `PATH`) |
| Python | packaging (`tools/sen_empaquetar.py`) | 3.9 to 3.14 (tried 3.9.6, 3.12.8 and 3.14.3); **standard library only** |
| git | clone the SDK | any |

No libc, no newlib, no operating system: the app is *freestanding* and only uses `libgcc` (shipped with the toolchain) for arithmetic the ARM926 does not do in hardware (e.g. division).

- **macOS**: download the *Arm GNU Toolchain* ("AArch32 bare-metal target, arm-none-eabi") from <https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>, unpack it and put its `bin/` in your `PATH`.
- **Linux**: Debian/Ubuntu `sudo apt install gcc-arm-none-eabi make python3 git`; Fedora `sudo dnf install arm-none-eabi-gcc-cs make python3 git`; or the same *Arm GNU Toolchain* as above.
- **Windows**: WSL with the Linux commands is the easiest. Native: *Arm GNU Toolchain* for Windows plus `make` and `python` (e.g. from MSYS2).

Check: `arm-none-eabi-gcc --version` and `python3 --version`.

> [!NOTE]
> Only the versions in the table have been tried. Others (e.g. Debian's) should work, but if linking fails because of `libgcc`, use the official *Arm GNU Toolchain*.

## 2. Build the template (five minutes)

```sh
git clone https://github.com/semakers/senew-sdk
cp -r senew-sdk/template my-app        # the template brings its own Makefile
cd my-app
make SENEW_SDK=../senew-sdk            # -> build/hello.sen
```

You will see something like:

```
build/hello.sen: «Hello» ARMv5TE perfil 1 API 1.7 | fichero 916 B | código 736, datos 0, bss 16, 13 reubicaciones, recursos 0 | pila 8192, montón 0 | RAM que toma: 9520 B
```

(The packager prints in Spanish: *fichero* = file, *código* = code, *datos* = data, *reubicaciones* = relocations, *recursos* = resources, *pila* = stack, *montón* = heap, *RAM que toma* = RAM it takes.)

If you put `my-app` next to `senew-sdk` a plain `make` is enough: the `Makefile` looks for the SDK in `../senew-sdk` and, if it is not there, clones it (shallow) into `./.senew-sdk`. Use `SENEW_SDK=/path` to pick another copy.

**Put it on the phone**: open **SENUS** with the phone connected, go to **Apps** and use **"Subir .sen"** (upload a .sen). The app shows up in the menu (or under "Apps" if the menu is full).
**Try it in the emulator first**: see [§8](#8-trying-it-in-the-emulator).

## 3. Anatomy of an app

The template (`template/src/hello.c`) is a counter; this is the whole app:

```c
#include "w5.h"
#include "w5_pantalla.h"     /* (API 1.7) the new font; gives W5_PANT_MENOR */

W5_APP(.nombre = "Hello", .version = 1, .api_menor = W5_PANT_MENOR, .caps = W5_CAP_GIRA, .icono_gen = W5_IC_RELOJ);

static int count;            /* the app's state: plain statics (each launch starts cold) */

static int key(W5Ctx c, int k, void *dato)
{
    (void)dato;
    if (k == W5K_OK) count++;
    else if (k == W5K_C) count = 0;
    else return ui_teclas_defecto(c, k);   /* right soft key = back; the rest: system defaults */
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
    return 0;
}
```

(Identifiers are Spanish: *suceso* = event, *abrir* = open, *pantalla* = screen, *andamio* = scaffold/frame, *sucio* = dirty, *teclas* = keys, *columna* = column, *hijos* = children, *texto* = text.)

The pieces:

1. **`W5_APP(...)`**: the app's identity: name (the menu label, UTF-8, up to 29 bytes), version, the **API it needs** (`.api_menor`), the **capabilities** it requires (`.caps`), the generic system icon (`.icono_gen`), stack (`.pila`, default 8 KiB) and heap (`.monton`, for `ap_mem_pedir`). Only the packager reads it: it takes no RAM on the phone.
2. **`app_suceso()`**: the **only** function the system calls. `W5_ABRIR` returns the root screen; `W5_PAUSA`/`W5_SIGUE` tell the app it stopped being visible (a system alert on top, screen off) and is back; `W5_SALIR` is the last chance to save.
3. **Screens**: `ui_pantalla(construir, iniciar, desechar, dato)`. The system calls `construir` ("build") every time something changed and you return the **widget tree**. `iniciar` and `desechar` (init/dispose) are optional: that is where timers are started and stopped.
4. **Widgets**: `Columna`, `Fila`, `Texto`, `Lista`, `Caja`… with `A_*` attributes (see [`sdk-api.md`](sdk-api.md) §4). **A widget lives for one frame**: never store them. State goes in your variables.
5. **Keys**: wrap the screen in `Teclas(handler, child)`; the handler returns 1 if it takes the key. `ui_teclas_defecto` does the standard thing (right soft key or "back" = leave).
6. **`ui_andamio(title, body, left, right)`**: the standard frame: title bar, your body and the labels of the two soft keys.

### 3.1. Time: never wait in a loop

The app has **no main loop of its own** (except the game profile): the system calls it for events and timers. A `while (!ready) ;` inside the app **never ends**, because nothing moves while your code has not returned.

```c
static void every_second(W5Ctx c) { ui_sucio(c); }
static void init(W5Ctx c, void *dato) { (void)dato; ui_cada(c, 1000, every_second); }   /* dies with the screen */
```

`ui_cada(ctx, ms, f)` repeats, `ui_tras(ctx, ms, f)` fires once, `ui_cancelar` removes them. Timers die by themselves when the screen closes and **do not tick while paused**. For a long computation call `ap_latido()` inside the loop (otherwise the watchdog closes you after ~30 s).
Everything slow (network, camera, microphone) is **asynchronous**: the call starts something and you check on it in a `ui_cada`.

### 3.2. Memory

- **Stack**: 4 KiB to 256 KiB (`.pila`); default 8 KiB. Do not put big arrays on the stack.
- **Statics** (`.data`, `.bss`): count against the app's RAM. To see how much, read the last column the packager prints ("RAM que toma").
- **Heap**: request it with `W5_APP(.monton = …)` and use `ap_mem_pedir(n)` / `ap_mem_soltar` / `ap_mem_libre` (up to 14 MiB; zero-filled; `NULL` if it does not fit). There is no `malloc`.
- **Per-frame arena**: `ui_fmt()` and `ui_arena(n)` return memory that **lasts one frame**. To keep a string, copy it into a buffer of your own.
- Large read-only **resources** (levels, sounds, textures) go **at the end of the `.sen`** (`--rec file`, which `mk/app.mk` passes automatically if `resources.bin` exists) and take no RAM: read them in chunks with `ap_rec_leer`, or in place with `ap_rec_mapa` (may return `NULL`).
- No floating-point hardware: use integers or fixed point (`senew-app-calculadora` is 4-decimal fixed point). The compiler can emit `libgcc` calls for floats, but they are slow.

### 3.3. Files

- `ap_fich_leer/escribir/borrar/tam(name, …)`: the app's **own** files. The system prefixes them with the app's name, so you can neither see nor overwrite other apps' files. A write is atomic (the whole file is saved) and requires `W5_CAP_FICHEROS`. There is a cap of 20 writes per minute (flash wear).
- `fs_tam/fs_leer/fs_listar`: **read-only** access to any file or folder on the phone (memory and card).
- `ma_*`: "My files", the user's folder (e.g. saving a note as `.txt`).

### 3.4. Capabilities and API version

Each device says what it has (`ap_consulta(W5_Q_CAPS)`: accelerometer, vibrator, lights, joystick, numeric keypad…) and how big its screen is (`W5_Q_ANCHO/ALTO`, width/height). If you put a capability in `.caps` the app **only loads** where it exists; if it is optional, do not request it and ask at run time. Widgets re-layout by themselves; **declare `W5_CAP_GIRA`** if your UI copes with rotating with the accelerometer.

`.api_menor` is the minor version of the API you build against: the system refuses (with a clear message) an app that asks for more than it has. Ask for **the minimum**: each `w5_*.h` header documents its minor (`W5_PANT_MENOR` = 7, `W5_CAM_MENOR` = 9…). Optional groups (network, music over the network, camera, microphone, sound, downloads) are probed before use: `w5_red_hay()`, `w5_mured_hay()`…

### 3.5. Icons

- `.icono_gen = W5_IC_*`: a generic system icon (calculator, clock, notes, game…).
- `icon/arte.txt` (optional): 12 lines of the same length (1 to 16 characters), `#` = ink; the flat 1-bit icon stored in the `.sen` header. `mk/app.mk` uses it only if it exists.
- `icon/malla.bin` (optional): a 3D mesh (up to 256 triangles) for the menu's 3D styles. Its format is in `include/cargable.h` (`W5xIcoMalla`); the packager validates it exactly as the phone does.

## 4. Project layout

```
my-app/
  Makefile        APP = my_app + the part that finds the SDK + include $(SENEW_SDK)/mk/app.mk
  src/*.c, *.h    everything is compiled together (src/ is on the include path)
  icon/arte.txt   optional
  icon/malla.bin  optional
  resources.bin   optional (read-only resources)
```

Useful `Makefile` variables (before the `include`): `APP`, `SRCS`, `EXTRA_SRCS`, `EXTRA_CFLAGS` (e.g. `-O2`), `ICON_ART`, `ICON_MESH`, `RES`, `ARMCC`, `PYTHON`, `BUILD`. All are documented in [`mk/app.mk`](../../mk/app.mk). Targets: `make`, `make size`, `make clean`.

The compiler flags are fixed and best left alone: `-mcpu=arm926ej-s -mfloat-abi=soft -marm -mthumb-interwork -ffreestanding -fno-builtin -fno-pic -Os …`. If you want Thumb to save code, compile your sources with `-mthumb` (`EXTRA_CFLAGS = -mthumb`): the system calls the app with interworking. What you **cannot** use is PIC (`-fpic`): the app is linked at address 0 and the system relocates it by adding the base to a list of words.

## 5. The packager: from ELF to `.sen`

`tools/sen_empaquetar.py app.elf app.sen [--rec resources.bin] [--arte icon.txt] [--malla mesh.bin] [--api MAJOR.MINOR]` reads the ELF (linked at 0 with `lib/w5_app.ld` and `--emit-relocs`), checks that **every** relocation is "absolute word" or "PC-relative", builds the image (code, data, bss), the relocation list, the resources and the 128-byte header with its CRCs, and writes the `.sen`.
The hard decisions are made here, on your computer: if something cannot be relocated by the phone, **packaging fails** with a message (for instance "compile without -fpic and for ARMv5"), not the phone. It can also change the icon of an existing `.sen`: `sen_empaquetar.py --poner-icono in.sen out.sen --arte icon.txt`.

The exact format (the `W5xCab` header, limits, CRCs) is in `include/cargable.h` §1.

## 6. Common errors

| Message | Cause and fix |
|---|---|
| `falta W5_APP(...) en la app (sección .w5meta)` | You forgot the `W5_APP(...)` line (or that file was not compiled). |
| `reubicación de tipo N … no se puede llevar a un .sen` | You compiled with PIC or for another architecture; use the flags in `mk/app.mk`. |
| `la palabra en 0x… apunta fuera de la app` | A pointer to a fixed address (e.g. a hardware register): an app does not touch hardware, use the API. |
| `undefined reference to 'memcpy'` etc. | `w5_crt0.c` defines them: make sure `mk/app.mk` links it (do not use a different Makefile without copying it). |
| The app appears in the menu but "closes" on open with a message | Read the message: missing capability, API newer than the system's, stack overflow, pointer outside the app… |
| `Pide la API 1.N y este sistema tiene la 1.M` ("asks for API 1.N, this system has 1.M") | Lower `.api_menor` or update senew from SENUS. |
| A `ui_fmt` string "gets corrupted" later | It lasts one frame: copy it. |

## 7. Real examples

Each repo builds with this SDK and is an app that ships (or shipped) with the system or is published in SENUS's store:

| You want to… | Look at |
|---|---|
| The basics: state, keys, a timer, the time | `senew-app-ejemplo` ("Cuentavueltas", lap counter), `template/` |
| A list, dialogs, text entry, own files | `senew-app-notas` |
| A game on top of the widget loop | `senew-app-serpiente` |
| Design-system components | `examples/componentes` (in this repo), `senew-app-calculadora`, `senew-app-luces` |
| Talk to the internet or your server | `senew-app-plantilla_red`, `senew-app-red`, and [`apps-con-internet.md`](apps-con-internet.md) |
| Music over the network / video | `senew-app-musica`, `senew-app-videos` |
| Camera / microphone / sound and vibrator | `senew-app-camara_prueba`, `senew-app-grabadora`, `senew-app-sonido` |
| Accelerometer, steps, raw keypad, serial port | `senew-app-movimiento`, `senew-app-teclas`, `senew-app-consola` |
| Images (GIF/PNG/JPEG) | `senew-app-imagenes` |
| A big game (game profile: full screen, own loop, large heap) | `senew-doom` |

## 8. Trying it in the emulator

senew runs on a QEMU model of the W580i: [`senew-emu`](https://github.com/semakers/senew-emu). The system (`senew-w580`) builds an emulator flash image with `make w580` (`build/flash-w580.bin`, **for the emulator only**: never flash it to a phone). The exact steps to preload a `.sen` into the emulated memory and drive it with the keypad are in the [`senew-w580`](https://github.com/semakers/senew-w580) README, section "Trying an app in the emulator".
You can also try the system in the browser at <https://semakers.com/probar/> (the official emulator; it does not accept your own apps).

## 9. Trying it on the phone

1. Install senew with SENUS (see <https://semakers.com>), **with a backup done**.
2. SENUS → **Apps** → **Subir .sen**. To remove it, the same screen.
3. A failing app **does not bring the phone down**: the system closes it and says why ("App cerrada: reason"). What can happen is that it runs slowly or drains the battery: test with care.

## 10. SDK versions and compatibility

The system API is `1.N` and **only grows at the end**: an app built today keeps working on future systems of the same major. `api/cargable_v1.lst` is the frozen list of the first functions; `tools/hacer_api.py cargable.h --comprobar api/cargable_v1.lst` checks that none moved. To regenerate `include/w5_nombres.h` (the short names) after touching `cargable.h`: `python3 tools/hacer_api.py include/cargable.h --nombres include/w5_nombres.h`.

## 11. License and contributions

The SDK and the apps are **MIT**. The system (`senew-w580`) and SENUS are GPL-3.0; a `.sen` app does not link against them (it only talks through the table), so **your app can carry whatever license you like**.
Issues and pull requests are welcome at <https://github.com/semakers/senew-sdk>. The comments in the headers are in Spanish, the project's working language; this guide and [`sdk-api.md`](sdk-api.md) also exist in Spanish (`docs/es/`).
