#include "i8042.h"

.globl _ass
.text
_ass:
xor %eax,%eax
KBD6:
inb $STAT_REG
test $OUT_BUF_FULL, %al
loopz KBD6
inb $OUT_BUF
ret
