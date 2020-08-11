```asm
0000000000400efc <phase_2>:
  400efc:	55                   	push   %rbp
  400efd:	53                   	push   %rbx
  400efe:	48 83 ec 28          	sub    $0x28,%rsp
  400f02:	48 89 e6             	mov    %rsp,%rsi
  400f05:	e8 52 05 00 00       	callq  40145c <read_six_numbers>
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)
  400f0e:	74 20                	je     400f30 <phase_2+0x34>
  400f10:	e8 25 05 00 00       	callq  40143a <explode_bomb>
  400f15:	eb 19                	jmp    400f30 <phase_2+0x34>
  400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax
  400f1a:	01 c0                	add    %eax,%eax
  400f1c:	39 03                	cmp    %eax,(%rbx)
  400f1e:	74 05                	je     400f25 <phase_2+0x29>
  400f20:	e8 15 05 00 00       	callq  40143a <explode_bomb>
  400f25:	48 83 c3 04          	add    $0x4,%rbx
  400f29:	48 39 eb             	cmp    %rbp,%rbx
  400f2c:	75 e9                	jne    400f17 <phase_2+0x1b>
  400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>
  400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx
  400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp
  400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>
  400f3c:	48 83 c4 28          	add    $0x28,%rsp
  400f40:	5b                   	pop    %rbx
  400f41:	5d                   	pop    %rbp
  400f42:	c3                   	retq   
```

1. randomly input 6 numbers and pause before `400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)`, use `(gdb) print *(int*) ($rsp)`, `(gdb) print *(int*) ($rsp+0x4)`,`(gdb) print *(int*) ($rsp+0x8)`, ..., could know that 6 numbers are in `$rsp+i`, for `i` in range `[0, 5]`

2. 
    ```asm
    400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)
    400f0e:	74 20                	je     400f30 <phase_2+0x34>
    ```
    means 1st number should be `1`

3. then jump to
   ```asm
   400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx
   400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp
   ```
   result to $rbx hold 2nd number, and $rbp is the first stack address after the 6th number (cuz 6th number is at `0x14(%rsp)`)
4. then we jump to
    ```asm
    400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax
    400f1a:	01 c0                	add    %eax,%eax
    400f1c:	39 03                	cmp    %eax,(%rbx)
    400f1e:	74 05                	je     400f25 <phase_2+0x29>
    ```
    we know $rbx points to a number, say _i_ th , so `-0x4(%rbx)` points to _i-1_ th number. This code segment basically compares two adjacent numbers, the latter should be twice as the former
5. then jump to
    ```asm
    400f25:	48 83 c3 04          	add    $0x4,%rbx
    400f29:	48 39 eb             	cmp    %rbp,%rbx
    ```
    this moves forward a number, and check if reach the last one. If not, continue the step4 above (check twice equal). If so, finish.
6. so the solution is `1 2 4 8 16 32`