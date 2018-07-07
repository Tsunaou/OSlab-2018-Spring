.set PROT_MODE_CSEG, 0x8         # kernel code segment selector    
.set PROT_MODE_DSEG, 0x10        # kernel data segment selector
.set PROT_MODE_VSEG, 0x18        # kernel vedio segment selector

.code16

.global start
start:
	cli                             #关闭中断
	inb $0x92, %al                  #启动A20总线
	orb $0x02, %al
	outb %al, $0x92
	data32 addr32 lgdt gdtDesc      #加载GDTR
	movl %cr0, %eax                 #启动保护模式
	orb $0x01, %al
	movl %eax, %cr0
	data32 ljmp $0x08, $start32     #长跳转切换至保护模式

.code32
start32:
		
	# Set up the protected-mode data segment registers

	##define GDT_ENTRY(n) ((n) << 3)	
	movw    $PROT_MODE_DSEG, %ax
	movw    %ax, %ds          # %DS = %AX
	movw    %ax, %es          # %ES = %AX
	movw    %ax, %ss          # %SS = %AX
	movw	%ax, %fs		  # %FS = %FX
	movw 	$PROT_MODE_VSEG, %ax
	movw	%ax, %gs		  # %GS = %AX
	
	# Set up a stack for C code.
	movl $0, %ebp
	movl $(128 << 20), %esp
	# sub $16, %esp



	jmp bootMain                    #跳转至bootMain函数 定义于boot.c

	

.p2align 2
gdt:
	.word 0,0 # empty entry
	.byte 0,0,0,0

	.word 0xffff,0                  #代码段描述符
	.byte 0,0x9a,0xcf,0
        
	.word 0xffff,0                  #数据段描述符
	.byte 0,0x92,0xcf,0
        
	.word 0xffff,0x8000             #视频段描述符
	.byte 0x0b,0x92,0xcf,0
    	#...

gdtDesc:
	.word (gdtDesc - gdt -1)
	.long gdt
