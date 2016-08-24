	.text
	.file	"/mnt/example.c"
	.globl	run
	.p2align	2
	.type	run,@function
run:                                    ! @run
! BB#0:                                 ! %entry
	st	%fp, [--%sp]
	add	%sp, 0x8, %fp
	sub	%sp, 0x18, %sp
	or	%r6, 0x0, %r3
	st	%r6, -12[%fp]
	st	%r0, -16[%fp]
	st	%r0, -20[%fp]
	bt	.LBB0_1
	st	%r3, -24[%fp]
.LBB0_1:                                ! %for.cond
                                        ! =>This Inner Loop Header: Depth=1
	ld	 -20[%fp], %r3
	ld	 -12[%fp], %r9
	ld.b	[%r9 add %r3], %r3
	sub.f	%r3, 0x0, %r0
	beq	.LBB0_4
	nop
	bt	.LBB0_2
	nop
.LBB0_2:                                ! %for.body
                                        !   in Loop: Header=BB0_1 Depth=1
	ld	 -20[%fp], %r3
	ld	 -12[%fp], %r9
	bt	.LBB0_3
	st.b	%r3, [%r9 add %r3]
.LBB0_3:                                ! %for.inc
                                        !   in Loop: Header=BB0_1 Depth=1
	ld	 -20[%fp], %r3
	add	%r3, 0x1, %r3
	bt	.LBB0_1
	st	%r3, -20[%fp]
.LBB0_4:                                ! %for.end
	ld	-4[%fp], %pc ! return
	add	%fp, 0x0, %sp
	ld	 -8[%fp], %fp
.Lfunc_end0:
	.size	run, .Lfunc_end0-run


	.ident	"clang version 4.0.0 (http://llvm.org/git/clang.git 656204ffb45bbf056101265d3ae4811638184c17) (http://llvm.org/git/llvm.git c662b7eae3c4ffd44ce42e85024d94015ac5b08a)"
	.section	".note.GNU-stack","",@progbits
