#include <inc/mmu.h>

.set PROT_MODE_CSEG, 0x8         # kernel code segment selector
.set PROT_MODE_DSEG, 0x10        # kernel data segment selector
.set CR0_PE_ON,      0x1         # protected mode enable flag

.globl start
start:
	.code16                     # Assemble for 16-bit mode

	# Set up the important data segment registers (DS, ES, SS).
	xorw    %ax,%ax             # Segment number zero
	movw    %ax,%ds             # -> Data Segment
	movw    %ax,%es             # -> Extra Segment
	movw    %ax,%ss             # -> Stack Segment

	cli                         # Disable interrupts
	cld                         # String operations increment

	# Switch from real to protected mode, using a bootstrap GDT
	# and segment translation that makes virtual addresses 
	# identical to their physical addresses, so that the 
	# effective memory map does not change during the switch.
	lgdt    gdtdesc
	movl    %cr0, %eax
	orl     $CR0_PE_ON, %eax
	movl    %eax, %cr0

	# Jump to next instruction, but in 32-bit code segment.
	# Switches processor into 32-bit mode.
	ljmp    $PROT_MODE_CSEG, $protcseg

.code32                     # Assemble for 32-bit mode
protcseg:
	# Set up the protected-mode data segment registers
	movw    $PROT_MODE_DSEG, %ax    # Our data segment selector
	movw    %ax, %ds                # -> DS: Data Segment
	movw    %ax, %es                # -> ES: Extra Segment
	movw    %ax, %fs                # -> FS
	movw    %ax, %gs                # -> GS
	movw    %ax, %ss                # -> SS: Stack Segment

	# Set up the stack pointer and call into C.
	  movl    $start, %esp
	  call bootmain
loop:
	jmp loop

# Bootstrap GDT
	.p2align 2                                # force 4 byte alignment
gdt:
# TODO: Fillup GDT entry with SEG macro
#       One for kernel code segment
#       One for kernel data segment
#
#       Please reference lab3_reference.pdf for the meaning of
#       each bit inside segment descriptor.
#
#       You may reference mmu.h for Application Segment Type for 
#       easy-to-use macro when defining segmentation type.

	SEG_NULL    # NULL segment
	SEG(STA_X | STA_R , 0, 0x7ff << 12)     # Kernel code segment
	SEG(STA_W, 0, 0x7ff << 12)     # Kernel data segment

# TODO: Please fillup the GDT register for gdt in order to used
#       with "lgdt" instruction.
# Note: You can reference lab3_reference page 13 for detail meaning
#       of each bit of GDT register.
gdtdesc: 
	.word	gdtdesc - gdt - 1 # gdt limit
	.long	gdt # gdt base
