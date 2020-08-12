```asm
000000000040100c <phase_4>:
  40100c:	48 83 ec 18          	sub    $0x18,%rsp
  401010:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
  401015:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
  40101a:	be cf 25 40 00       	mov    $0x4025cf,%esi
  40101f:	b8 00 00 00 00       	mov    $0x0,%eax
  401024:	e8 c7 fb ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
  401029:	83 f8 02             	cmp    $0x2,%eax
  40102c:	75 07                	jne    401035 <phase_4+0x29>
  40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)
  401033:	76 05                	jbe    40103a <phase_4+0x2e>
  401035:	e8 00 04 00 00       	callq  40143a <explode_bomb>
  40103a:	ba 0e 00 00 00       	mov    $0xe,%edx
  40103f:	be 00 00 00 00       	mov    $0x0,%esi
  401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi
  401048:	e8 81 ff ff ff       	callq  400fce <func4>
  40104d:	85 c0                	test   %eax,%eax
  40104f:	75 07                	jne    401058 <phase_4+0x4c>
  401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)
  401056:	74 05                	je     40105d <phase_4+0x51>
  401058:	e8 dd 03 00 00       	callq  40143a <explode_bomb>
  40105d:	48 83 c4 18          	add    $0x18,%rsp
  401061:	c3                   	retq  
```

1. before the call of `func4`, this assembly code is very similar to phase_3: input two int numbers, and store them at `0x8(%rsp)` and `0xc(%rsp)`
2. then we progress to
   ```asm
   40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)
   401033:	76 05                	jbe    40103a <phase_4+0x2e>
   401035:	e8 00 04 00 00       	callq  40143a <explode_bomb>
   ```
   it compares the first int with `14`, and continues only if the first int is less or equal to `14`. And because it is `jbe`, so it must be `0 <= first_int <= 14`
3. then it sets a global variable in `edx`, with initial value to be `14`, progress to call `func4`. It passes two arguments to `func4`, in `edi`, `esi`, respectively. We shoud notice that `edi` holds the first int
4. exam `func4` assembly code
   ```asm
    0000000000400fce <func4>:
    400fce:	48 83 ec 08          	sub    $0x8,%rsp
    400fd2:	89 d0                	mov    %edx,%eax
    400fd4:	29 f0                	sub    %esi,%eax
    400fd6:	89 c1                	mov    %eax,%ecx
    400fd8:	c1 e9 1f             	shr    $0x1f,%ecx
    400fdb:	01 c8                	add    %ecx,%eax
    400fdd:	d1 f8                	sar    %eax
    400fdf:	8d 0c 30             	lea    (%rax,%rsi,1),%ecx
    400fe2:	39 f9                	cmp    %edi,%ecx
    400fe4:	7e 0c                	jle    400ff2 <func4+0x24>
    400fe6:	8d 51 ff             	lea    -0x1(%rcx),%edx
    400fe9:	e8 e0 ff ff ff       	callq  400fce <func4>
    400fee:	01 c0                	add    %eax,%eax
    400ff0:	eb 15                	jmp    401007 <func4+0x39>
    400ff2:	b8 00 00 00 00       	mov    $0x0,%eax
    400ff7:	39 f9                	cmp    %edi,%ecx
    400ff9:	7d 0c                	jge    401007 <func4+0x39>
    400ffb:	8d 71 01             	lea    0x1(%rcx),%esi
    400ffe:	e8 cb ff ff ff       	callq  400fce <func4>
    401003:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax
    401007:	48 83 c4 08          	add    $0x8,%rsp
    40100b:	c3                   	retq 
   ```
   it could be translate to this pseudo code
   ```python
   eax = edx - esi
   ecx = eax >> 31 # logical shift
   eax = eax + ecx
   eax = eax / 2 # arithmetic shift by 1 is equal to divide by 2
   ecx = eax + esi

   if ecx <= edi:
      if ecx >= edi:
        return 0
      esi += (ecx+1)
      return 2 * func4(edi, esi) + 1
    else:
      edx = ecx - 1
      return 2 * func4(edi, esi)
   ```
   it can be simplified into
   ```python
   eax = edx - esi
   ecx = eax >> 31 # logical shift
   eax = eax + ecx
   eax = eax / 2 # arithmetic shift by 1 is equal to divide by 2
   ecx = eax + esi

   if ecx == edi:
      return 0
   else if ecx < edi:
      b += (ecx+1)
      return 2 * func4(edi, esi) + 1
   else:
      edx = ecx - 1
      return 2 * func4(edi, esi)
   ```
   so in order to pass `phase_4`, we should let `func4` return `0`
5. with the initial value being `edx = 14`, `esi = 0`, we could step through each instruction

   | instruction | result |
   | - | - |
   | `eax = edx - esi` | `eax = 14` |
   | `ecx = eax >> 31` | `eax = 14, ecx = 0` |
   | `eax = eax + ecx` | `eax = 14, ecx = 0` |
   | `eax = eax / 2` | `eax = 7, ecx = 0` |
   | `ecx = eax + esi` | `eax = 7, ecx = 7` |

   so in order to pass `func4` in its first run, `edi` should be `7`, so is the first int
6. return back to `phase_4`
   ```asm
   401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)
   401056:	74 05                	je     40105d <phase_4+0x51>
   ```
   it compares the second int with `0`, because of `je` comparison, the second int must be `0` to pass
7. so the solution is `7 0`