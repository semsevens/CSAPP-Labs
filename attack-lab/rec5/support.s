	.file "support.s"
	.text
	.global clobber
	.type clobber, @function
clobber:
	.cfi_startproc
    mov %rsi, %rax
    mov %rsp, %rcx
.loop:
    movq (%rdi), %rdx
    movq %rdx, (%rcx)
    add $8, %rdi
    add $8, %rcx
    sub $8, %rax
    jnz .loop
	ret
    call *%rsi
    .size clobber, .-clobber
	.cfi_endproc

