# File generated by CompCert 2.4
# Command line:
	.text
	.align	16
	.globl ditio_main
ditio_main:
	.cfi_startproc
	subl	$20, %esp
	.cfi_adjust_cfa_offset	20
	leal	24(%esp), %edx
	movl	%edx, 0(%esp)
	movl	%ebx, 4(%esp)
	movl	%esi, 8(%esp)
	movl	0(%edx), %esi
	movl	8(%edx), %ebx
	movzbl	0(%ebx), %eax
	movzbl	1(%ebx), %ecx
	movzbl	2(%ebx), %edx
	movzbl	3(%ebx), %ebx
	cmpl	$100, %ebx
	jb	.L100
	cmpl	$4, %eax
	jne	.L100
	cmpl	$48, %ecx
	jne	.L100
	cmpl	$48, %ecx
	sete	%bl
	movzbl	%bl, %ebx
	cmpl	$1, %edx
	sete	%al
	movzbl	%al, %eax
	andl	%eax, %ebx
	movzbl	48(%esi), %edx
	testl	$255, %edx
	sete	%al
	movzbl	%al, %eax
	andl	%eax, %ebx
	cmpl	$1, %edx
	sete	%cl
	movzbl	%cl, %ecx
	orl	%ecx, %ebx
	cmpl	$0, %ebx
	je	.L100
	movl	$196608, %eax
	jmp	.L101
.L100:
	xorl	%eax, %eax
.L101:
	movl	4(%esp), %ebx
	movl	8(%esp), %esi
	addl	$20, %esp
	ret
	.cfi_endproc
	.type	ditio_main, @function
	.size	ditio_main, . - ditio_main
