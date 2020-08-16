```asm
0000000000400ee0 <phase_1>:
  400ee0:	48 83 ec 08          	sub    $0x8,%rsp
  400ee4:	be 00 24 40 00       	mov    $0x402400,%esi
  400ee9:	e8 4a 04 00 00       	callq  401338 <strings_not_equal>
  400eee:	85 c0                	test   %eax,%eax
  400ef0:	74 05                	je     400ef7 <phase_1+0x17>
  400ef2:	e8 43 05 00 00       	callq  40143a <explode_bomb>
  400ef7:	48 83 c4 08          	add    $0x8,%rsp
  400efb:	c3                   	retq   
```

1. `(gdb) call ((int(*)()) strings_not_equal)("1", "1")`  
    get result: `$1 = 0`  
    so in order to pass `je`, `strings_not_equal` should result 0, which means `(gdb) print (char *) ($rdi)` should equal to `(gdb) print (char *) ($rsi)`
2. `(gdb) break phase_1`, stepi to `callq <strings_not_equal>`, exam `(gdb) print (char *) ($rsi)` will get result:  
    > Border relations with Canada have never been better.
3. so is the solution to phase_1