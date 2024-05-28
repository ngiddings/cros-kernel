.section ".text"

.global _start
_start:
    bl main
    b _start
