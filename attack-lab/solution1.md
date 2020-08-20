1. use `objdump -d ctarget` to get assembly code
2. from assembly code, get address of `touch1` is `00000000004017c0`
3. disassemble `getbuf`, get this piece of code:
   ```asm
    4017a8:	48 83 ec 28          	sub    $0x28,%rsp
    4017ac:	48 89 e7             	mov    %rsp,%rdi
    4017af:	e8 8c 02 00 00       	callq  401a40 <Gets>
   ```
   since our input string stores at `$rsp`, so in order to overwrite the return address, we need to fulfill `0x28 + 8 = 48` bytes of string

    the first 40 bytes is useless, so they can be random bytes, the last 8 bytes should be the start address of `touch1`, for `ret` instruction in `getbuf` to jump to `touch1`

    so the last 8 bytes should be `c0 17 40 00 00 00 00 00` (little endian byte ordering)

    plus a terminate byte: `0x0a` to terminate string input