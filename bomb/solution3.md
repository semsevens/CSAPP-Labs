```asm
0000000000400f43 <phase_3>:
  400f43:	48 83 ec 18          	sub    $0x18,%rsp
  400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
  400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax
  400f5b:	e8 90 fc ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
  400f60:	83 f8 01             	cmp    $0x1,%eax
  400f63:	7f 05                	jg     400f6a <phase_3+0x27>
  400f65:	e8 d0 04 00 00       	callq  40143a <explode_bomb>
  400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)
  400f6f:	77 3c                	ja     400fad <phase_3+0x6a>
  400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax
  400f75:	ff 24 c5 70 24 40 00 	jmpq   *0x402470(,%rax,8)
  400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
  400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax
  400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
  400f8a:	b8 00 01 00 00       	mov    $0x100,%eax
  400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>
  400f91:	b8 85 01 00 00       	mov    $0x185,%eax
  400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
  400f98:	b8 ce 00 00 00       	mov    $0xce,%eax
  400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
  400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax
  400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
  400fa6:	b8 47 01 00 00       	mov    $0x147,%eax
  400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
  400fad:	e8 88 04 00 00       	callq  40143a <explode_bomb>
  400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
  400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
  400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax
  400fc2:	74 05                	je     400fc9 <phase_3+0x86>
  400fc4:	e8 71 04 00 00       	callq  40143a <explode_bomb>
  400fc9:	48 83 c4 18          	add    $0x18,%rsp
  400fcd:	c3                   	retq 
```

1. first exam that
   ```asm
   400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi
   ```
   `$esi` is the second argument to `int sscanf( const char          *buffer, const char          *format, ... );`, so we know `0x4025cf` holds the `format`
2. use `(gdb) x/s 0x4025cf`, we get `"%d %d"`, which means phase_3 accepts two int arguments
3. we can exam that `0x8(%rsp)` holds the first int, and `0xc(%rsp)` holds the second int
4. after `sscanf` and check its return value, we jump to
   ```asm
   400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)
   400f6f:	77 3c                	ja     400fad <phase_3+0x6a>
   ```
   this compares the first int with `7`, if the first int is greater than `7`, then bomb explodes.
5. then we progress to
   ```asm
   400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax
   400f75:	ff 24 c5 70 24 40 00 	jmpq   *0x402470(,%rax,8)
   ```
   this use the first int as index to indirectly jump to somewhere. This is a clue to "switch structure".  
   We can use the range `[0, 7]` to get the following table  

   | i | `(gdb) x/wx (0x402470+i*8)` | corresponding `mov` src | `(gdb) print /d src` to get `mov` source in decimal |
   | - | - | - | - |
   | 0 | 0x400f7c | 0xcf | 207 |
   | 1 | 0x400fb9 | 0x137 | 311 |
   | 2 | 0x400f83 | 0x2c3 | 707 |
   | 3 | 0x400f8a | 0x100 | 256 |
   | 4 | 0x400f91 | 0x185 | 389 |
   | 5 | 0x400f98 | 0xce | 206 |
   | 6 | 0x400f9f | 0x2aa | 682 |
   | 7 | 0x400fa6 | 0x147 | 327 |
6. all case jump to this instruction
   ```asm
   400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax
   ```
   this compares the second int with each switch case result `$eax`, defuse bomb only if they are equal
7. pseudo code
   ```python
   first_int, second_int = input_two_ints()

   switch(first_int):
      case 0:
        if second_int != 207:
          explode_bomb()
        break
      case 1:
        if second_int != 311:
          explode_bomb()
        break
      case 2:
        if second_int != 707:
          explode_bomb()
        break
      case 3:
        if second_int != 256:
          explode_bomb()
        break
      case 4:
        if second_int != 389:
          explode_bomb()
        break
      case 5:
        if second_int != 206:
          explode_bomb()
        break
      case 6:
        if second_int != 682:
          explode_bomb()
        break
      case 7:
        if second_int != 327:
          explode_bomb()
        break
      default:
        explode_bomb()
   ```