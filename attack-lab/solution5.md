the key point is to construct byte representation of `59b997fa` in stack and **make %rdi point to the address of stack**

since there is no gadget available that can store stack address and **jump over one element in stack** (which is `popq`) simultaneously, the byte representation can only be constructed outside the first `ret` address and the call to `touch3`

notice there is a special gadget `4019d6:	48 8d 04 37          	lea    (%rdi,%rsi,1),%rax`, it can be used to calculate address. Also, use gdb, we can observe that `%rsi` is always `0x30`

see [rtarget.l5.txt](./rtarget.l5.txt) with detailed comment