/* w5_juego.ld — guion de enlace de un JUEGO GRANDE (api 1.1): el de w5_app.ld más la VENTANA DE RECURSOS.
 * La imagen (código, datos, bss) sigue enlazada en 0 y se reubica al cargar, como siempre. Los datos de solo lectura que
 * tools/sen_recursos.py ha elegido (rec.ld: lo que solo apunta a otros datos de solo lectura) se enlazan en W5_REC_VA
 * (0x70000000, include/cargable.h): el empaquetador los pone al final del .sen como RECURSOS y el sistema los mapea ahí.
 * Lo que va en .w5rec NO puede apuntar a la imagen (no se reubica: está en la flash); el empaquetador lo comprueba. */
ENTRY(_w5_entrada)
SECTIONS
{
    .w5rec 0x70000000 : { w5_rec_ini = .; KEEP(*(.w5rec.firma)) INCLUDE rec.ld . = ALIGN(16); w5_rec_fin = .; }
    .text 0 : { KEEP(*(.w5entrada)) *(.text .text.*) *(.glue_7 .glue_7t .v4_bx) *(.rodata .rodata.*) . = ALIGN(16); }
    .data : { *(.data .data.*) *(.got .got.*) . = ALIGN(16); }
    .bss (NOLOAD) : { *(.bss .bss.* COMMON) . = ALIGN(16); }
    .w5meta 0 (INFO) : { KEEP(*(.w5meta)) }
    /DISCARD/ : { *(.comment) *(.ARM.attributes) *(.ARM.exidx*) *(.ARM.extab*) *(.note*) *(.eh_frame*) *(.interp) *(.dynamic) *(.dynsym) *(.dynstr) *(.hash) *(.gnu.hash) *(.rela.dyn) *(.rel.dyn) *(.iplt*) *(.igot*) }
}
