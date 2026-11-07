/* w5_app.ld — guion de enlace de una app cargable: TODO seguido desde la dirección 0 (la imagen se mueve entera).
 * Orden: entrada, código, constantes | datos | bss. El empaquetador (tools/sen_empaquetar.py) saca de aquí los tamaños. */
ENTRY(_w5_entrada)
SECTIONS
{
    . = 0;
    .text : { KEEP(*(.w5entrada)) *(.text .text.*) *(.glue_7 .glue_7t .v4_bx) *(.rodata .rodata.*) . = ALIGN(16); }
    .data : { *(.data .data.*) *(.got .got.*) . = ALIGN(16); }
    .bss (NOLOAD) : { *(.bss .bss.* COMMON) . = ALIGN(16); }
    .w5meta 0 (INFO) : { KEEP(*(.w5meta)) }
    /DISCARD/ : { *(.comment) *(.ARM.attributes) *(.ARM.exidx*) *(.ARM.extab*) *(.note*) *(.eh_frame*) *(.interp) *(.dynamic) *(.dynsym) *(.dynstr) *(.hash) *(.gnu.hash) *(.rela.dyn) *(.rel.dyn) *(.iplt*) *(.igot*) }
}
