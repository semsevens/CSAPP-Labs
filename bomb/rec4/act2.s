	.file "act2.s"
	.text
	.global stc
	.type stc, @function
stc:
	.cfi_startproc
    push %rbx
    mov %rsi, %rbx
	call puts
    mov %rbx, %rdi
    call puts
    pop %rbx
	ret
	.cfi_endproc

