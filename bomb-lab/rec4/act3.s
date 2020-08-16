    .file    "act2b.s"
    .text
    .globl    compare
    .type    compare, @function
compare:
.LFB0:
    .cfi_startproc
    pushq     %rbx
    movq      %rdi, %rbx
    addq      $5, %rbx
    addq      %rsi, %rbx
    cmpq      $15213, %rbx
    sete      %al
    movzbq    %al, %rax
    popq      %rbx
    ret
    .cfi_endproc
.LFE0:
    .size    compare, .-compare
    .ident    "GCC: (GNU) 4.8.1"
    .section    .note.GNU-stack,"",@progbits
