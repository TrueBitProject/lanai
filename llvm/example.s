	.text
	.file	"/mnt/example.c"
	.globl	mod
	.p2align	2
	.type	mod,@function
mod:                                    ! @mod
! BB#0:                                 ! %entry
	st	%fp, [--%sp]
	add	%sp, 0x8, %fp
	sub	%sp, 0x18, %sp
	or	%r7, 0x0, %r3
	or	%r6, 0x0, %r9
	st	%r6, -12[%fp]
	st	%r7, -16[%fp]
	st	%r3, -20[%fp]
	bt	.LBB0_1
	st	%r9, -24[%fp]
.LBB0_1:                                ! %while.cond
                                        ! =>This Inner Loop Header: Depth=1
	ld	 -12[%fp], %r3
	ld	 -16[%fp], %r9
	sub.f	%r3, %r9, %r0
	bult	.LBB0_3
	nop
	bt	.LBB0_2
	nop
.LBB0_2:                                ! %while.body
                                        !   in Loop: Header=BB0_1 Depth=1
	ld	 -16[%fp], %r3
	ld	 -12[%fp], %r9
	sub	%r9, %r3, %r3
	bt	.LBB0_1
	st	%r3, -12[%fp]
.LBB0_3:                                ! %while.end
	ld	 -12[%fp], %rv
	ld	-4[%fp], %pc ! return
	add	%fp, 0x0, %sp
	ld	 -8[%fp], %fp
.Lfunc_end0:
	.size	mod, .Lfunc_end0-mod

	.globl	prime
	.p2align	2
	.type	prime,@function
prime:                                  ! @prime
! BB#0:                                 ! %entry
	st	%fp, [--%sp]
	add	%sp, 0x8, %fp
	sub	%sp, 0x340, %sp
	or	%r6, 0x0, %r3
	st	%r6, -12[%fp]
	mov	0x2, %r6
	st	%r6, -16[%fp]
	st	%r0, -24[%fp]
	st	%r6, -828[%fp]
	bt	.LBB1_1
	st	%r3, -832[%fp]
.LBB1_1:                                ! %while.cond
                                        ! =>This Loop Header: Depth=1
                                        !     Child Loop BB1_3 Depth 2
	ld	 -24[%fp], %r3
	ld	 -12[%fp], %r9
	sub.f	%r3, %r9, %r0
	bge	.LBB1_11
	nop
	bt	.LBB1_2
	nop
.LBB1_2:                                ! %while.body
                                        !   in Loop: Header=BB1_1 Depth=1
	mov	0x2, %r3
	bt	.LBB1_3
	st	%r3, -20[%fp]
.LBB1_3:                                ! %for.cond
                                        !   Parent Loop BB1_1 Depth=1
                                        ! =>  This Inner Loop Header: Depth=2
	ld	 -20[%fp], %r3
	ld	 -16[%fp], %r9
	sub	%r9, 0x1, %r9
	sub.f	%r3, %r9, %r0
	bgt	.LBB1_8
	nop
	bt	.LBB1_4
	nop
.LBB1_4:                                ! %for.body
                                        !   in Loop: Header=BB1_3 Depth=2
	ld	 -16[%fp], %r6
	add	%pc, 0x10, %rca
	st	%rca, [--%sp]
	bt	mod
	ld	 -20[%fp], %r7
	sub.f	%rv, 0x0, %r0
	bne	.LBB1_6
	nop
	bt	.LBB1_5
	nop
.LBB1_5:                                ! %if.then
                                        !   in Loop: Header=BB1_1 Depth=1
	bt	.LBB1_8
	nop
.LBB1_6:                                ! %if.end
                                        !   in Loop: Header=BB1_3 Depth=2
	bt	.LBB1_7
	nop
.LBB1_7:                                ! %for.inc
                                        !   in Loop: Header=BB1_3 Depth=2
	ld	 -20[%fp], %r3
	add	%r3, 0x1, %r3
	bt	.LBB1_3
	st	%r3, -20[%fp]
.LBB1_8:                                ! %for.end
                                        !   in Loop: Header=BB1_1 Depth=1
	ld	 -20[%fp], %r3
	ld	 -16[%fp], %r9
	sub.f	%r3, %r9, %r0
	bne	.LBB1_10
	nop
	bt	.LBB1_9
	nop
.LBB1_9:                                ! %if.then4
                                        !   in Loop: Header=BB1_1 Depth=1
	ld	 -16[%fp], %r3
	st	%r3, -828[%fp]
	ld	 -24[%fp], %r9
	add	%r9, 0x1, %r12
	st	%r12, -24[%fp]
	sh	%r9, 0x2, %r9
	sub	%fp, 0x338, %r12
	bt	.LBB1_10
	st	%r3, [%r12 add %r9]
.LBB1_10:                               ! %if.end6
                                        !   in Loop: Header=BB1_1 Depth=1
	ld	 -16[%fp], %r3
	add	%r3, 0x1, %r3
	bt	.LBB1_1
	st	%r3, -16[%fp]
.LBB1_11:                               ! %while.end
	ld	 -828[%fp], %rv
	ld	-4[%fp], %pc ! return
	add	%fp, 0x0, %sp
	ld	 -8[%fp], %fp
.Lfunc_end1:
	.size	prime, .Lfunc_end1-prime

	.globl	simple
	.p2align	2
	.type	simple,@function
simple:                                 ! @simple
! BB#0:                                 ! %entry
	st	%fp, [--%sp]
	add	%sp, 0x8, %fp
	sub	%sp, 0x10, %sp
	or	%r6, 0x0, %r3
	st	%r6, -12[%fp]
	add	%r6, 0x14, %rv
	st	%r3, -16[%fp]
	ld	-4[%fp], %pc ! return
	add	%fp, 0x0, %sp
	ld	 -8[%fp], %fp
.Lfunc_end2:
	.size	simple, .Lfunc_end2-simple

	.globl	compare
	.p2align	2
	.type	compare,@function
compare:                                ! @compare
! BB#0:                                 ! %entry
	st	%fp, [--%sp]
	add	%sp, 0x8, %fp
	sub	%sp, 0x18, %sp
	or	%r7, 0x0, %r3
	or	%r6, 0x0, %r9
	st	%r6, -12[%fp]
	st	%r7, -16[%fp]
	ld	 -12[%fp], %r6
	sub.f	%r6, %r7, %r0
	suge	%rv
	st	%r3, -20[%fp]
	st	%r9, -24[%fp]
	ld	-4[%fp], %pc ! return
	add	%fp, 0x0, %sp
	ld	 -8[%fp], %fp
.Lfunc_end3:
	.size	compare, .Lfunc_end3-compare

	.globl	main
	.p2align	2
	.type	main,@function
main:                                   ! @main
! BB#0:                                 ! %entry
	st	%fp, [--%sp]
	add	%sp, 0x8, %fp
	sub	%sp, 0x10, %sp
	st	%r0, -12[%fp]
	mov	0x22, %r6
	add	%pc, 0x10, %rca
	st	%rca, [--%sp]
	bt	mod
	mov	0x3, %r7
	ld	-4[%fp], %pc ! return
	add	%fp, 0x0, %sp
	ld	 -8[%fp], %fp
.Lfunc_end4:
	.size	main, .Lfunc_end4-main


	.ident	"clang version 4.0.0 (http://llvm.org/git/clang.git 656204ffb45bbf056101265d3ae4811638184c17) (http://llvm.org/git/llvm.git c662b7eae3c4ffd44ce42e85024d94015ac5b08a)"
	.section	".note.GNU-stack","",@progbits
