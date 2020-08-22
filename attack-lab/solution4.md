use gadget farm to get this instruction sequence:

```asm
popq %rax
nop
ret
movq %rax, %rdi
ret              # ret to call `touch2`
```

see [rtarget.l4.txt](./rtarget.l4.txt) for detail