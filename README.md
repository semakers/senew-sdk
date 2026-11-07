# senew-sdk — build apps (`.sen`) for senew

*Español más abajo — [ir a la versión en español](#español).*

> [!WARNING]
> **senew is a new project; use it at your own risk.** The system, this SDK and the tools are young software for a discontinued phone. Back up the phone before installing anything and try apps in the emulator first.

**senew** is an open operating system for the Sony Ericsson W580i ([github.com/semakers/senew-w580](https://github.com/semakers/senew-w580), site: <https://semakers.com>).
Apps for it are small ARM files with the extension **`.sen`**. This SDK has everything you need to write, build and package one **outside** the senew tree:

| Folder | What |
|---|---|
| `include/` | The public API: `cargable.h` (the contract: file format + function table, API 1.0 to 1.3) and `w5_*.h` (optional groups: sound, network, screen, music over the network, camera, microphone, downloads), `w5.h` (what an app includes), `w5_nombres.h` (short names, generated) |
| `lib/` | `w5_crt0.c` (start-up code), `w5_app.ld` (linker script), and for big games `w5_juego.c/.ld` |
| `tools/` | `sen_empaquetar.py` (ELF → `.sen`: header, relocations, icon, resources, CRCs), `hacer_api.py` (regenerate/check the API list), `sen_recursos.py` and `sen_ausentes.py` (helpers for porting large programs/games) |
| `mk/app.mk` | The build rules for an app: `include` it from a 5-line Makefile |
| `template/` | A minimal app that compiles (`make SENEW_SDK=..`) |
| `examples/componentes/` | Showcase of the design-system components |
| `docs/en`, `docs/es` | Guide ([`sdk.md`](docs/en/sdk.md)), API reference ([`sdk-api.md`](docs/en/sdk-api.md)), apps that talk to the internet ([`apps-con-internet.md`](docs/en/apps-con-internet.md)) |
| `api/cargable_v1.lst` | The frozen list of the first 77 functions (the API only grows at the end) |

## Quick start

```sh
# 1. Toolchain: Arm GNU Toolchain (arm-none-eabi-gcc) 15.x, GNU make, Python 3.8+, git
git clone https://github.com/semakers/senew-sdk
cp -r senew-sdk/template my-app && cd my-app
make SENEW_SDK=../senew-sdk          # -> build/hello.sen
```

Then open **SENUS** (<https://github.com/semakers/senus>) with the phone connected → **Apps** → **"Subir .sen"**.
Real, complete apps that build with this SDK: [`senew-app-*`](https://github.com/semakers?tab=repositories&q=senew-app) (calculator, notes, camera, network demo, music…).

Tested toolchains: Arm GNU Toolchain 15.2.Rel1 (macOS arm64) and Fedora's `arm-none-eabi-gcc-cs` 15.2.0. The packager is pure-stdlib Python (3.9.6, 3.12.8 and 3.14.3 tried).

## Documentation

- [Guide: your first app from scratch](docs/en/sdk.md)
- [API reference](docs/en/sdk-api.md)
- [Apps that talk to their own backend (through SENUS)](docs/en/apps-con-internet.md)

The comments inside the headers and the sources are in Spanish (the project's working language); the documents above are in English and in Spanish.

## License

MIT — see [`LICENSE`](LICENSE). senew itself and SENUS are GPL-3.0, but a `.sen` app only talks to the system through the function table and does not link against it, so **your app can use any license**.
Sony Ericsson, W580i and other names are trademarks of their respective owners and are used only to say which hardware this is about; this project is not affiliated with them.

---

<a id="español"></a>

# senew-sdk — haz apps (`.sen`) para senew

> [!WARNING]
> **senew es un proyecto nuevo; úsalo bajo tu propio riesgo.** El sistema, este SDK y las herramientas son software joven para un teléfono descatalogado. Haz copia de seguridad del teléfono antes de instalar nada y prueba las apps primero en el emulador.

**senew** es un sistema operativo abierto para el Sony Ericsson W580i ([github.com/semakers/senew-w580](https://github.com/semakers/senew-w580), web: <https://semakers.com>).
Sus apps son ficheros ARM pequeños con la extensión **`.sen`**. Este SDK trae todo lo necesario para escribir, compilar y empaquetar una **fuera** del árbol de senew:

| Carpeta | Qué |
|---|---|
| `include/` | La API pública: `cargable.h` (el contrato: formato del fichero + tabla de funciones, API 1.0 a 1.3) y `w5_*.h` (grupos opcionales: sonido, red, pantalla, música por la red, cámara, micrófono, descargas), `w5.h` (lo que incluye una app), `w5_nombres.h` (nombres cortos, generado) |
| `lib/` | `w5_crt0.c` (arranque), `w5_app.ld` (guion de enlace) y, para juegos grandes, `w5_juego.c/.ld` |
| `tools/` | `sen_empaquetar.py` (ELF → `.sen`: cabecera, reubicaciones, icono, recursos, CRC), `hacer_api.py` (regenera/comprueba la lista de la API), `sen_recursos.py` y `sen_ausentes.py` (ayudas para portar programas/juegos grandes) |
| `mk/app.mk` | Las reglas para compilar una app: se incluye desde un Makefile de 5 líneas |
| `template/` | Una app mínima que compila (`make SENEW_SDK=..`) |
| `examples/componentes/` | Muestrario de los componentes del sistema de diseño |
| `docs/en`, `docs/es` | Guía ([`sdk.md`](docs/es/sdk.md)), referencia de la API ([`sdk-api.md`](docs/es/sdk-api.md)), apps que hablan con internet ([`apps-con-internet.md`](docs/es/apps-con-internet.md)) |
| `api/cargable_v1.lst` | La lista congelada de las 77 primeras funciones (la API solo crece por el final) |

## Empezar

```sh
# 1. Cadena de herramientas: Arm GNU Toolchain (arm-none-eabi-gcc) 15.x, GNU make, Python 3.8+, git
git clone https://github.com/semakers/senew-sdk
cp -r senew-sdk/template mi-app && cd mi-app
make SENEW_SDK=../senew-sdk          # -> build/hello.sen
```

Después abre **SENUS** (<https://github.com/semakers/senus>) con el teléfono conectado → **Apps** → **«Subir .sen»**.
Apps reales y completas que compilan con este SDK: [`senew-app-*`](https://github.com/semakers?tab=repositories&q=senew-app) (calculadora, notas, cámara, demo de red, música…).

Cadenas probadas: Arm GNU Toolchain 15.2.Rel1 (macOS arm64) y `arm-none-eabi-gcc-cs` 15.2.0 de Fedora. El empaquetador es Python con solo la biblioteca estándar (probado en 3.9.6, 3.12.8 y 3.14.3).

## Documentación

- [Guía: tu primera app de principio a fin](docs/es/sdk.md)
- [Referencia de la API](docs/es/sdk-api.md)
- [Apps que hablan con su propio backend (a través de SENUS)](docs/es/apps-con-internet.md)

Los comentarios de las cabeceras y de los fuentes están en español (la lengua de trabajo del proyecto); los documentos de arriba están en español y en inglés.

## Licencia

MIT — ver [`LICENSE`](LICENSE). senew y SENUS son GPL-3.0, pero una app `.sen` solo habla con el sistema por la tabla de funciones y no enlaza con él, así que **tu app puede llevar la licencia que quieras**.
Sony Ericsson, W580i y otros nombres son marcas de sus respectivos dueños y se usan solo para decir de qué aparato se trata; este proyecto no tiene relación con ellos.
