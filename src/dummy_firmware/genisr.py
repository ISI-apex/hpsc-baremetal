#!/usr/bin/python

import argparse

def int_list(s):
    return map(int, s.split(','))

parser = argparse.ArgumentParser(
    description="Generate assembly source for vector table")
parser.add_argument('--internal-irqs', type=int, default=16,
    help='Number internal IRQs')
parser.add_argument('--external-irqs', type=int, default=240,
   help='Number external IRQs')
parser.add_argument('--handlers', type=int_list,
    help='Number external IRQs')
parser.add_argument('out_asm',
    help='Output file with generated C source')
parser.add_argument('out_c',
    help='Output file with generated assembly source')
args = parser.parse_args()

if args.handlers is None:
        args.handlers = range(0, 240)

def external(irq):
	return irq - args.internal_irqs

# ISR handlers for each vector number
# The rest of the vectors (not in this dict) get default handler
DEFAULT_ISR = "hang"
isr = {
 0: None,
 1: "reset",
11: "svc",
}

f = open(args.out_asm, "w")

f.write(
"""
.cpu cortex-m4
.thumb

.thumb_func
.global _start
_start:
stacktop: .word 0x20001000
"""
)

for i in range(0, args.internal_irqs + args.external_irqs):
    handler = None
    if i in isr:
        if isr[i] is not None:
            handler = isr[i]
    elif external(i) in args.handlers:
	handler = "isr%u" % external(i)
    else:
        handler = DEFAULT_ISR

    if handler is not None:
        f.write(".word %s\n" % handler)

f.write("\n")

f.write(
"""
.thumb_func
reset:
    bl notmain
    b hang
    b hang

.thumb_func
svc:
    mov r0, #0
    sub r0, #7 // 0xfffffff9: priveledged Thread mode with main stack
    bx r0

.thumb_func
hang:   b .
    b hang
"""
+ "\n");

for irq in args.handlers:
    f.write((".thumb_func\n" + \
             "isr%u:\n" + \
	     "    push {lr}\n" + \
             "    bl c_isr%u\n" + \
             "    pop {pc}\n") % (irq, irq))

# Generate C source for stub IRQ handlers (ISRs)

f = open(args.out_c, "w")

f.write(
"""
/* This file was automatically generated by genisr.py.
 *
 * The following define stub functions that are called by the IRQ handlers
 * defined in assembly in vectors.s (generated by genvec.py).
 */
""")

f.write(
"""
#include <libmspprintf/printf.h>
""")

for irq in args.handlers:
    f.write(
"""
int c_isr%u (void) {
    static unsigned num_invoc = 0;
    void *p = 0x0;
    asm ("mov %%0, lr\\n" : "=r" (p));
    printf("IRQ %u (%%lu): LR %%p\\n", num_invoc, p);
    num_invoc++;
    return(0);
}
""" % (irq, irq))
