gdb disassemble of `phase_6`:

```asm
0x00000000004010f4 <+0>:     push   %r14
0x00000000004010f6 <+2>:     push   %r13
0x00000000004010f8 <+4>:     push   %r12
0x00000000004010fa <+6>:     push   %rbp
0x00000000004010fb <+7>:     push   %rbx
0x00000000004010fc <+8>:     sub    $0x50,%rsp
0x0000000000401100 <+12>:    mov    %rsp,%r13
0x0000000000401103 <+15>:    mov    %rsp,%rsi
0x0000000000401106 <+18>:    callq  0x40145c <read_six_numbers>
0x000000000040110b <+23>:    mov    %rsp,%r14
0x000000000040110e <+26>:    mov    $0x0,%r12d
0x0000000000401114 <+32>:    mov    %r13,%rbp
0x0000000000401117 <+35>:    mov    0x0(%r13),%eax
0x000000000040111b <+39>:    sub    $0x1,%eax
0x000000000040111e <+42>:    cmp    $0x5,%eax
0x0000000000401121 <+45>:    jbe    0x401128 <phase_6+52>
0x0000000000401123 <+47>:    callq  0x40143a <explode_bomb>
0x0000000000401128 <+52>:    add    $0x1,%r12d
0x000000000040112c <+56>:    cmp    $0x6,%r12d
0x0000000000401130 <+60>:    je     0x401153 <phase_6+95>
0x0000000000401132 <+62>:    mov    %r12d,%ebx
0x0000000000401135 <+65>:    movslq %ebx,%rax
0x0000000000401138 <+68>:    mov    (%rsp,%rax,4),%eax
0x000000000040113b <+71>:    cmp    %eax,0x0(%rbp)
0x000000000040113e <+74>:    jne    0x401145 <phase_6+81>
0x0000000000401140 <+76>:    callq  0x40143a <explode_bomb>
0x0000000000401145 <+81>:    add    $0x1,%ebx
0x0000000000401148 <+84>:    cmp    $0x5,%ebx
0x000000000040114b <+87>:    jle    0x401135 <phase_6+65>
0x000000000040114d <+89>:    add    $0x4,%r13
0x0000000000401151 <+93>:    jmp    0x401114 <phase_6+32>
0x0000000000401153 <+95>:    lea    0x18(%rsp),%rsi
0x0000000000401158 <+100>:   mov    %r14,%rax
0x000000000040115b <+103>:   mov    $0x7,%ecx
0x0000000000401160 <+108>:   mov    %ecx,%edx
0x0000000000401162 <+110>:   sub    (%rax),%edx
0x0000000000401164 <+112>:   mov    %edx,(%rax)
0x0000000000401166 <+114>:   add    $0x4,%rax
0x000000000040116a <+118>:   cmp    %rsi,%rax
0x000000000040116d <+121>:   jne    0x401160 <phase_6+108>
0x000000000040116f <+123>:   mov    $0x0,%esi
0x0000000000401174 <+128>:   jmp    0x401197 <phase_6+163>
0x0000000000401176 <+130>:   mov    0x8(%rdx),%rdx
0x000000000040117a <+134>:   add    $0x1,%eax
0x000000000040117d <+137>:   cmp    %ecx,%eax
0x000000000040117f <+139>:   jne    0x401176 <phase_6+130>
0x0000000000401181 <+141>:   jmp    0x401188 <phase_6+148>
0x0000000000401183 <+143>:   mov    $0x6032d0,%edx
0x0000000000401188 <+148>:   mov    %rdx,0x20(%rsp,%rsi,2)
0x000000000040118d <+153>:   add    $0x4,%rsi
0x0000000000401191 <+157>:   cmp    $0x18,%rsi
0x0000000000401195 <+161>:   je     0x4011ab <phase_6+183>
0x0000000000401197 <+163>:   mov    (%rsp,%rsi,1),%ecx
0x000000000040119a <+166>:   cmp    $0x1,%ecx
0x000000000040119d <+169>:   jle    0x401183 <phase_6+143>
0x000000000040119f <+171>:   mov    $0x1,%eax
0x00000000004011a4 <+176>:   mov    $0x6032d0,%edx
0x00000000004011a9 <+181>:   jmp    0x401176 <phase_6+130>
0x00000000004011ab <+183>:   mov    0x20(%rsp),%rbx
0x00000000004011b0 <+188>:   lea    0x28(%rsp),%rax
0x00000000004011b5 <+193>:   lea    0x50(%rsp),%rsi
0x00000000004011ba <+198>:   mov    %rbx,%rcx
0x00000000004011bd <+201>:   mov    (%rax),%rdx
0x00000000004011c0 <+204>:   mov    %rdx,0x8(%rcx)
0x00000000004011c4 <+208>:   add    $0x8,%rax
0x00000000004011c8 <+212>:   cmp    %rsi,%rax
0x00000000004011cb <+215>:   je     0x4011d2 <phase_6+222>
0x00000000004011cd <+217>:   mov    %rdx,%rcx
0x00000000004011d0 <+220>:   jmp    0x4011bd <phase_6+201>
0x00000000004011d2 <+222>:   movq   $0x0,0x8(%rdx)
0x00000000004011da <+230>:   mov    $0x5,%ebp
0x00000000004011df <+235>:   mov    0x8(%rbx),%rax
0x00000000004011e3 <+239>:   mov    (%rax),%eax
0x00000000004011e5 <+241>:   cmp    %eax,(%rbx)
0x00000000004011e7 <+243>:   jge    0x4011ee <phase_6+250>
0x00000000004011e9 <+245>:   callq  0x40143a <explode_bomb>
0x00000000004011ee <+250>:   mov    0x8(%rbx),%rbx
0x00000000004011f2 <+254>:   sub    $0x1,%ebp
0x00000000004011f5 <+257>:   jne    0x4011df <phase_6+235>
0x00000000004011f7 <+259>:   add    $0x50,%rsp
0x00000000004011fb <+263>:   pop    %rbx
0x00000000004011fc <+264>:   pop    %rbp
0x00000000004011fd <+265>:   pop    %r12
0x00000000004011ff <+267>:   pop    %r13
0x0000000000401201 <+269>:   pop    %r14
0x0000000000401203 <+271>:   retq
```

1. `phase_6` accepts six numbers as arguments, say it `ints`
2. from
   ```asm
    0x0000000000401117 <+35>:    mov    0x0(%r13),%eax
    0x000000000040111b <+39>:    sub    $0x1,%eax
    0x000000000040111e <+42>:    cmp    $0x5,%eax
    0x0000000000401121 <+45>:    jbe    0x401128 <phase_6+52>
    ```
    we know that `1 <= first int <= 6`
3. then we progress to
    ```asm
    0x0000000000401114 <+32>:    mov    %r13,%rbp
    0x0000000000401117 <+35>:    mov    0x0(%r13),%eax
    0x000000000040111b <+39>:    sub    $0x1,%eax
    0x000000000040111e <+42>:    cmp    $0x5,%eax
    0x0000000000401121 <+45>:    jbe    0x401128 <phase_6+52>
    0x0000000000401123 <+47>:    callq  0x40143a <explode_bomb>
    0x0000000000401128 <+52>:    add    $0x1,%r12d
    0x000000000040112c <+56>:    cmp    $0x6,%r12d
    0x0000000000401130 <+60>:    je     0x401153 <phase_6+95>
    0x0000000000401132 <+62>:    mov    %r12d,%ebx
    0x0000000000401135 <+65>:    movslq %ebx,%rax
    0x0000000000401138 <+68>:    mov    (%rsp,%rax,4),%eax
    0x000000000040113b <+71>:    cmp    %eax,0x0(%rbp)
    0x000000000040113e <+74>:    jne    0x401145 <phase_6+81>
    0x0000000000401140 <+76>:    callq  0x40143a <explode_bomb>
    0x0000000000401145 <+81>:    add    $0x1,%ebx
    0x0000000000401148 <+84>:    cmp    $0x5,%ebx
    0x000000000040114b <+87>:    jle    0x401135 <phase_6+65>
    0x000000000040114d <+89>:    add    $0x4,%r13
    0x0000000000401151 <+93>:    jmp    0x401114 <phase_6+32>
    ```
    notice that `<phase_6+65>` and `<phase_6+32>`, it forms a two-phase loop. Express in pseudo code should be:
    ```python
    for i in range(0, 6):
      if ints[i] < 1 or ints[i] > 6:
        bomb()
      for j in range(i+1, 6):
        if ints[i] == ints[j]:
          bomb()
    ```
    so this piece of code check two things:
    
    - all ints are in range `0 <= int <= 6`
    - no two ints are same

    so we should input a permutation of `1 2 3 4 5 6`
4. then we progress to
    ```asm
    0x0000000000401153 <+95>:    lea    0x18(%rsp),%rsi
    0x0000000000401158 <+100>:   mov    %r14,%rax
    0x000000000040115b <+103>:   mov    $0x7,%ecx
    0x0000000000401160 <+108>:   mov    %ecx,%edx
    0x0000000000401162 <+110>:   sub    (%rax),%edx
    0x0000000000401164 <+112>:   mov    %edx,(%rax)
    0x0000000000401166 <+114>:   add    $0x4,%rax
    0x000000000040116a <+118>:   cmp    %rsi,%rax
    0x000000000040116d <+121>:   jne    0x401160 <phase_6+108>
    ```
    it also forms a loop:
    ```python
    for i in range(0, 6):
      ints[i] = 7 - ints[i]
    ```
    so this piece of code change each int to `7 - int`
5. then we progress to
    ```asm
    0x000000000040116f <+123>:   mov    $0x0,%esi
    0x0000000000401174 <+128>:   jmp    0x401197 <phase_6+163>
    0x0000000000401176 <+130>:   mov    0x8(%rdx),%rdx
    0x000000000040117a <+134>:   add    $0x1,%eax
    0x000000000040117d <+137>:   cmp    %ecx,%eax
    0x000000000040117f <+139>:   jne    0x401176 <phase_6+130>
    0x0000000000401181 <+141>:   jmp    0x401188 <phase_6+148>
    0x0000000000401183 <+143>:   mov    $0x6032d0,%edx
    0x0000000000401188 <+148>:   mov    %rdx,0x20(%rsp,%rsi,2)
    0x000000000040118d <+153>:   add    $0x4,%rsi
    0x0000000000401191 <+157>:   cmp    $0x18,%rsi
    0x0000000000401195 <+161>:   je     0x4011ab <phase_6+183>
    0x0000000000401197 <+163>:   mov    (%rsp,%rsi,1),%ecx
    0x000000000040119a <+166>:   cmp    $0x1,%ecx
    0x000000000040119d <+169>:   jle    0x401183 <phase_6+143>
    0x000000000040119f <+171>:   mov    $0x1,%eax
    0x00000000004011a4 <+176>:   mov    $0x6032d0,%edx
    0x00000000004011a9 <+181>:   jmp    0x401176 <phase_6+130>
    ```
    we can exam carefully
    - `0x00000000004011a4 <+176>:   mov    $0x6032d0,%edx`
    - `0x0000000000401176 <+130>:   mov    0x8(%rdx),%rdx`

    it is very interesting. The first instruction sets a base pointer, the second instruction reads the value next to location which the base pointer points to, and updates the pointer with this value, and repeats in a loop.

    **This forms a pattern of LINKED LIST**

    so this code is equal to pseudo code:

    ```python
    for i in range(0, 6):
      pointer = base # which is 0x6032d0

      if ints[i] <= 1:
        ptrs[i] = pointer
        break

      for j in range(1, 6):
        pointer = base.next
        if j+1 == ints[i]:
          ptrs[i] = pointer # ptrs starts at 0x20(%rsp)
          break
    ```

    so this piece of code use the permutation of `1 -> 6` as index, to get the corresponding linked list node, then store its address to `ptrs`

    now let us check the linked list starts at `0x6032d0`

    - use `(gdb) print *(int*) node.next` the get next `node.value` 
    - use `(gdb) print /x *(int*) (node.next+0x8)` to get next `node.next`

    we get the following table

    | ith | `node.value` | `node.next` |
    | - | - | - |
    | 1 | 322 | 0x6032e0 |
    | 2 | 168 | 0x6032f0 |
    | 3 | 924 | 0x603300 |
    | 4 | 691 | 0x603310 |
    | 5 | 477 | 0x603320 |
    | 6 | 443 | 0x0 (end of linked list) |
6. then we progress to
    ```asm
    0x00000000004011ab <+183>:   mov    0x20(%rsp),%rbx
    0x00000000004011b0 <+188>:   lea    0x28(%rsp),%rax
    0x00000000004011b5 <+193>:   lea    0x50(%rsp),%rsi
    0x00000000004011ba <+198>:   mov    %rbx,%rcx
    0x00000000004011bd <+201>:   mov    (%rax),%rdx
    0x00000000004011c0 <+204>:   mov    %rdx,0x8(%rcx)
    0x00000000004011c4 <+208>:   add    $0x8,%rax
    0x00000000004011c8 <+212>:   cmp    %rsi,%rax
    0x00000000004011cb <+215>:   je     0x4011d2 <phase_6+222>
    0x00000000004011cd <+217>:   mov    %rdx,%rcx
    0x00000000004011d0 <+220>:   jmp    0x4011bd <phase_6+201>
    ```
    notice these two interesting instructions:
    - `0x00000000004011c0 <+204>:   mov    %rdx,0x8(%rcx)`
    - `0x00000000004011cd <+217>:   mov    %rdx,%rcx`

    in the first iteration, `rcx` -> linked list head, `rdx` -> the second element chosen by input `ints`. So these two instructions links the chosen nodes into a new linked list
7. then we progress to
    ```asm
    0x00000000004011d2 <+222>:   movq   $0x0,0x8(%rdx)
    0x00000000004011da <+230>:   mov    $0x5,%ebp
    0x00000000004011df <+235>:   mov    0x8(%rbx),%rax
    0x00000000004011e3 <+239>:   mov    (%rax),%eax
    0x00000000004011e5 <+241>:   cmp    %eax,(%rbx)
    0x00000000004011e7 <+243>:   jge    0x4011ee <phase_6+250>
    0x00000000004011e9 <+245>:   callq  0x40143a <explode_bomb>
    0x00000000004011ee <+250>:   mov    0x8(%rbx),%rbx
    0x00000000004011f2 <+254>:   sub    $0x1,%ebp
    0x00000000004011f5 <+257>:   jne    0x4011df <phase_6+235>
    ```

    notice the first instruction
    ```asm
    0x00000000004011d2 <+222>:   movq   $0x0,0x8(%rdx)
    ```
    puts an end to the new linked list

    this piece of code also forms a loop:
    ```python
    node = base
    for i in range(5, 0, -1):
      if node.value >= node.next.value:
        pass
      else:
        bomb()

      node = node.next
    ```
8. combine all together, we can conclude that in order to defuse bomb, the new linked list should arrange in descending order of `node.value`. So the `ith` should be `3 4 5 6 1 2`, and `7 - ith` should be `4 3 2 1 6 5`
9. the solution is `4 3 2 1 6 5`