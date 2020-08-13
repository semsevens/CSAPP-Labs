```asm
0000000000401062 <phase_5>:
  401062:	53                   	push   %rbx
  401063:	48 83 ec 20          	sub    $0x20,%rsp
  401067:	48 89 fb             	mov    %rdi,%rbx
  40106a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
  401071:	00 00 
  401073:	48 89 44 24 18       	mov    %rax,0x18(%rsp)
  401078:	31 c0                	xor    %eax,%eax
  40107a:	e8 9c 02 00 00       	callq  40131b <string_length>
  40107f:	83 f8 06             	cmp    $0x6,%eax
  401082:	74 4e                	je     4010d2 <phase_5+0x70>
  401084:	e8 b1 03 00 00       	callq  40143a <explode_bomb>
  401089:	eb 47                	jmp    4010d2 <phase_5+0x70>
  40108b:	0f b6 0c 03          	movzbl (%rbx,%rax,1),%ecx
  40108f:	88 0c 24             	mov    %cl,(%rsp)
  401092:	48 8b 14 24          	mov    (%rsp),%rdx
  401096:	83 e2 0f             	and    $0xf,%edx
  401099:	0f b6 92 b0 24 40 00 	movzbl 0x4024b0(%rdx),%edx
  4010a0:	88 54 04 10          	mov    %dl,0x10(%rsp,%rax,1)
  4010a4:	48 83 c0 01          	add    $0x1,%rax
  4010a8:	48 83 f8 06          	cmp    $0x6,%rax
  4010ac:	75 dd                	jne    40108b <phase_5+0x29>
  4010ae:	c6 44 24 16 00       	movb   $0x0,0x16(%rsp)
  4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi
  4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi
  4010bd:	e8 76 02 00 00       	callq  401338 <strings_not_equal>
  4010c2:	85 c0                	test   %eax,%eax
  4010c4:	74 13                	je     4010d9 <phase_5+0x77>
  4010c6:	e8 6f 03 00 00       	callq  40143a <explode_bomb>
  4010cb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
  4010d0:	eb 07                	jmp    4010d9 <phase_5+0x77>
  4010d2:	b8 00 00 00 00       	mov    $0x0,%eax
  4010d7:	eb b2                	jmp    40108b <phase_5+0x29>
  4010d9:	48 8b 44 24 18       	mov    0x18(%rsp),%rax
  4010de:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax
  4010e5:	00 00 
  4010e7:	74 05                	je     4010ee <phase_5+0x8c>
  4010e9:	e8 42 fa ff ff       	callq  400b30 <__stack_chk_fail@plt>
  4010ee:	48 83 c4 20          	add    $0x20,%rsp
  4010f2:	5b                   	pop    %rbx
  4010f3:	c3                   	retq 
```

let us first ignore some memory segment check code, like `mov %fs:0x28,%rax`. Because it is only used to ensure stack access safety, these code can be safely ignored

```asm
0000000000401062 <phase_5>:
  401062:	53                   	push   %rbx
  401063:	48 83 ec 20          	sub    $0x20,%rsp
  401067:	48 89 fb             	mov    %rdi,%rbx

  401078:	31 c0                	xor    %eax,%eax
  40107a:	e8 9c 02 00 00       	callq  40131b <string_length>
  40107f:	83 f8 06             	cmp    $0x6,%eax
  401082:	74 4e                	je     4010d2 <phase_5+0x70>
  401084:	e8 b1 03 00 00       	callq  40143a <explode_bomb>
  401089:	eb 47                	jmp    4010d2 <phase_5+0x70>
  40108b:	0f b6 0c 03          	movzbl (%rbx,%rax,1),%ecx
  40108f:	88 0c 24             	mov    %cl,(%rsp)
  401092:	48 8b 14 24          	mov    (%rsp),%rdx
  401096:	83 e2 0f             	and    $0xf,%edx
  401099:	0f b6 92 b0 24 40 00 	movzbl 0x4024b0(%rdx),%edx
  4010a0:	88 54 04 10          	mov    %dl,0x10(%rsp,%rax,1)
  4010a4:	48 83 c0 01          	add    $0x1,%rax
  4010a8:	48 83 f8 06          	cmp    $0x6,%rax
  4010ac:	75 dd                	jne    40108b <phase_5+0x29>
  4010ae:	c6 44 24 16 00       	movb   $0x0,0x16(%rsp)
  4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi
  4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi
  4010bd:	e8 76 02 00 00       	callq  401338 <strings_not_equal>
  4010c2:	85 c0                	test   %eax,%eax
  4010c4:	74 13                	je     4010d9 <phase_5+0x77>
  4010c6:	e8 6f 03 00 00       	callq  40143a <explode_bomb>
  4010cb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
  4010d0:	eb 07                	jmp    4010d9 <phase_5+0x77>
  4010d2:	b8 00 00 00 00       	mov    $0x0,%eax
  4010d7:	eb b2                	jmp    40108b <phase_5+0x29>

  4010ee:	48 83 c4 20          	add    $0x20,%rsp
  4010f2:	5b                   	pop    %rbx
  4010f3:	c3                   	retq 
```

1. after initialization, `phase_5` call `string_length`. We can use `(gdb) call ((int(*)()) string_length)("1234")` to check that `string_length` indeed return the length of argument string
2. then `phase_5` check input string (the same string passed to `phase_5` as first argument, cuz `rdi` is unchanged)
   ```asm
    40107f:	83 f8 06             	cmp    $0x6,%eax
    401082:	74 4e                	je     4010d2 <phase_5+0x70>
   ```
   so the input string should be length `6`
3. then jump to
   ```asm
   4010d2:	b8 00 00 00 00       	mov    $0x0,%eax
   ```
   it initialize `eax` to `0`
4. then jump to the following assembly code, i put some comment after each instruction
   ```asm
    40108b:	0f b6 0c 03          	movzbl (%rbx,%rax,1),%ecx        # get the ith char in input str
    40108f:	88 0c 24             	mov    %cl,(%rsp)                # store that char byte in stack local variable area
    401092:	48 8b 14 24          	mov    (%rsp),%rdx               # also store in register rdx
    401096:	83 e2 0f             	and    $0xf,%edx                 # only get the lowest 4 bits
    401099:	0f b6 92 b0 24 40 00 	movzbl 0x4024b0(%rdx),%edx       # store a byte from memory to register edx
    4010a0:	88 54 04 10          	mov    %dl,0x10(%rsp,%rax,1)     # store that byte in ith stack local variable area above 0x10(%rsp)
    4010a4:	48 83 c0 01          	add    $0x1,%rax                 # increment i
    4010a8:	48 83 f8 06          	cmp    $0x6,%rax                 # continue loop for i < 6
    4010ac:	75 dd                	jne    40108b <phase_5+0x29>
    4010ae:	c6 44 24 16 00       	movb   $0x0,0x16(%rsp)           # add '\0' to the end of the string, which is generated by above loop. (C language uses '\0' to terminate string)
    4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi            # `(gdb) x/s 0x40245e` get "flyers", it's the second argument to `strings_not_equal` below
    4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi           # prepare the generated string as first argument to `strings_not_equal` below
    4010bd:	e8 76 02 00 00       	callq  401338 <strings_not_equal>
    4010c2:	85 c0                	test   %eax,%eax
    4010c4:	74 13                	je     4010d9 <phase_5+0x77>     # test string equal
   ```
    from the comments, we can conclude that this code snippet iterate each char of input 6-length string, use the lower 4 bits as bias of `0x4024b0` to get a byte(char) from memory, and concat these 6 translated char into a new string, then compare it with `flyers`
5. use `(gdb) x/s 0x4024b0`, we get `maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?`. But since bias is only 4 bits, so we just need to take the first 16 chars: `maduiersnfotvbyl`
6. in order to get char from `maduiersnfotvbyl` and translate to `flyers`, the bias should be `9, 15, 14, 5, 6, 7`, or `0x9, 0xf, 0xe, 0x5, 0x6, 0x7` respectively
7. check ASCII table and we can easily get the solution `IONEFG` (`0x49,0x4f,0x4e,0x45,0x46,0x47`), or `ionefg` (`0x69,0x6f,0x6e,0x65,0x66,0x67`)
