@
@ 090104 sunJH
@ ����Э��ջ����ʱ��ϳ�,��
@ ԭ���жϴ������̲�������,��˱�����е���
@ �����жϴ�������:
@ 1.�жϲ������룻
@ �������Ϊ��Э��ջʹ�õĶ�ʱ���жϲ������·�ʽ��
@     ��IRQģʽ����ģʽ����ǿռ����
@     ����Ĵ������ǳ��٣�ֻ����־�Ķ�������
@              ռ��ʱ��ǳ��̣�
@     IRQģʽ������ɺ󣬻��л���Systemģʽ��
@     Systemģʽ����ģʽ�ᱻ��ռ�������Ǹ����ȼ�������ȼ��ģ���
@              ����Ĵ������ϴ���������TCP/IPЭ�鹦�ܣ���
@              ռ��ʱ��ϳ��� 
@ 2.���ж���ڲ���SoftirqDo,
@  �ڸ��뺯�����л��Զ�enable irq���Ӷ�����irq nested
@ 090109 sunJH
@ �޸���Abort�������


.text

.EQU	FIQ_Stack,            0x40001000        /* 512 bytes  */        	
.EQU	IRQ_Stack,            0x40000E00        /* 3584 bytes */        
.EQU	SVC_Stack,            0x00200000        /* 256K bytes */
.EQU	ABT_Stack,            0x00208000        /* 32K bytes */
.EQU	UND_Stack,            0x0020F000        /* 32K bytes */
.EQU	USR_Stack,            0x00600000        /* 512K bytes */     
            
											  
	.extern  FIQ_Handler
	.global  FIQ_Handler
	.extern  IRQ_Handler
	.global  IRQ_Handler
	.extern  SWI_Handler
	.global  SWI_Handler
	.extern  Undefined_Handler
	.global  Undefined_Handler
	.extern  DataAbort_Handler
	.global  DataAbort_Handler
	.extern  PrefetchAbort_Handler
	.global  PrefetchAbort_Handler
	.extern  Get_Addr
	.global  Get_Addr 
	
	.extern  SWIHandler
	.extern  UndefinedHandler
	.extern  DataAbortHandler
	.extern  PrefetchAbortHandler
	.extern  FiqHandler
     .extern  main_code
	.global  MMU_Init 
	.extern  MMU_Init
    

   .global  __main
__main:

__entry:
        B       Reset_Handler
        B       Undefined_Handler
        B       SWI_Handler
        B       PrefetchAbort_Handler
        B       DataAbort_Handler
        NOP
        B       IRQ_Handler
        B       FIQ_Handler

Undefined_Handler:
        B       UndefinedHandler
PrefetchAbort_Handler:
        B       PrefetchAbortHandler         
DataAbort_Handler:
        B       DataAbortHandler

JUMP_Handler:
		BX      r3
		
FIQ_Handler:
		SUB     lr, lr, #4		
		STMFD   sp!, {r0-r3, lr} 
		LDR     r1, =FIQ_HANDLER
		LDR     r3, [r1,#0x00]
		BL      JUMP_Handler
        	LDMFD   sp!, {r0-r3, pc}^
                   
SWI_Handler:
        STMFD   sp!, {r0-r12,lr} 
		MOV     r1,sp
		MRS     r0,spsr
		STMFD   sp!,{r0}
        
		MRS     r1,cpsr
		BIC     r1,r1,#0xc0
		MSR     cpsr,r1


		LDR     r0,[lr,#-4]               /*ȡ���к�*/
		BIC     r0,r0,#0xFF000000
        
        BL      SWIHandler                      
        
		LDMFD   sp!,{r0}
		MSR     spsr_cf,r0
		
		LDMFD   sp!, {r0-r12,pc}^

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Added by sunJH
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	.global  softirq_bits

softirq_bits:
	.word	0
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ end by sunJH
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
           
IRQ_Handler:
        SUB     lr, lr, #4	
		STMFD   sp!, {r0-r3, r12, lr} 	
        MOV     r0, #0x4A000000     
        LDR     r1, [r0, #0x14] 
	LDR     r2, =IRQ_HANDLER_TABLE
        LDR     r3, [r2, r1, lsl #2] 
        BL      JUMP_Handler      				
        
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Added by sunJH
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @Firstly Save SPSR_irq
      	MRS      R0, SPSR
      	STMFD    SP!, {R0}                       @Store the SPSR
      	@Switch to System and disable I, enable F
      	MSR      cpsr_c, #0x9F
@
@ call SoftirqDo
@      	
        STMFD    SP!, {R0-R12,LR}
      	BL       SoftirqDo
        LDMFD    SP!, {R0-R12,LR}
      	@Switch to IRQ and disable I, enable F
      	MSR      cpsr_c, #0x92
      	@Restore SPSR_irq
      	LDMFD     SP!,{R0}            
      	MSR        SPSR_cf, R0         @restore SPSR
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ end by sunJH
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        LDMFD   sp!, {r0-r3, r12, pc}^       			


.EQU R1_C,       (1<<2)
.EQU R1_I,       (1<<12)
.EQU R1_M,       (1)
.EQU R1_A,       (1<<1)

.global MMU_EnableDCache
.extern MMU_EnableDCache 
MMU_EnableDCache:        
   mrc  p15,0,r0,c1,c0,0
   orr  r0,r0,#R1_C
   mcr  p15,0,r0,c1,c0,0
   MOV  pc,lr

.global MMU_DisableDCache
.extern MMU_DisableDCache 
MMU_DisableDCache:       
   mrc  p15,0,r0,c1,c0,0
   bic  r0,r0,#R1_C
   mcr  p15,0,r0,c1,c0,0
   MOV  pc,lr

.global  MMU_CleanInvalidateDCacheIndex
.extern  MMU_CleanInvalidateDCacheIndex
MMU_CleanInvalidateDCacheIndex:  
   mcr  p15,0,r0,c7,c14,2
   MOV  pc, lr

.global MMU_InvalidateDCache 
.extern MMU_InvalidateDCache 
MMU_InvalidateDCache:
   mcr  p15,0,r0,c7,c6,0
   MOV  pc, lr

.global MMU_EnableICache 
.extern MMU_EnableICache 
MMU_EnableICache:        
   mrc  p15,0,r0,c1,c0,0
   orr  r0,r0,#R1_I
   mcr  p15,0,r0,c1,c0,0
   MOV  pc, lr

.global  MMU_DisableICache
.extern  MMU_DisableICache
MMU_DisableICache:       
   mrc  p15,0,r0,c1,c0,0
   bic  r0,r0,#R1_I
   mcr  p15,0,r0,c1,c0,0
   mov  pc,lr

.global  MMU_InvalidateICache
.extern  MMU_InvalidateICache
MMU_InvalidateICache:
   mcr  p15,0,r0,c7,c5,0
   mov  pc,lr

.global  MMU_SetDomain
.extern  MMU_SetDomain
MMU_SetDomain:
   mcr  p15,0,r0,c3,c0,0
   MOV pc, lr

.global  MMU_SetTTBase
.extern  MMU_SetTTBase
MMU_SetTTBase:
   mcr  p15,0,r0,c2,c0,0
   mov  pc,lr

.global  MMU_SetAsyncBusMode
.extern  MMU_SetAsyncBusMode
MMU_SetAsyncBusMode:
   mrc  p15,0,r0,c1,c0,0
   orr  r0,r0,#0xC0000000
   mcr  p15,0,r0,c1,c0,0
   mov  pc,lr


.global  MMU_EnableMMU
.extern  MMU_EnableMMU
MMU_EnableMMU:
   mrc  p15,0,r0,c1,c0,0
   orr  r0,r0,#R1_M
   mcr  p15,0,r0,c1,c0,0
   mov  pc,lr

.global  MMU_DisableMMU
.extern  MMU_DisableMMU
MMU_DisableMMU:
   mrc  p15,0,r0,c1,c0,0
   bic  r0,r0,#R1_M
   mcr  p15,0,r0,c1,c0,0
   mov  pc,lr
   
.global  MMU_EnableAlignFault 
.extern  MMU_EnableAlignFault
MMU_EnableAlignFault:
   mrc  p15,0,r0,c1,c0,0
   orr  r0,r0,#R1_A
   mcr  p15,0,r0,c1,c0,0
   mov  pc,lr

.global  MMU_InvalidateTLB
.extern  MMU_InvalidateTLB
MMU_InvalidateTLB:      
   mcr  p15,0,r0,c8,c7,0
   MOV  pc,lr
   
.global  MMU_SetProcessId
.extern  MMU_SetProcessId   
MMU_SetProcessId:        
   mcr  p15,0,r0,c13,c0,0
   MOV  pc,lr
 

         
Reset_Handler:

        MOV     R0, #0xd1
        MSR     CPSR_c, R0
        LDR     R13, =FIQ_Stack     
        MOV     R0, #0xd2
        MSR     CPSR_c, R0
        LDR     R13, =IRQ_Stack 			           			
        MOV     R0, #0xd3
        MSR     CPSR_c, R0
        LDR     R13, =SVC_Stack 			
        MOV     R0, #0xd7
        MSR     CPSR_c, R0
        LDR     R13, =ABT_Stack 			
        MOV     R0, #0xdb
        MSR     CPSR_c, R0
        LDR     R13, =UND_Stack			
        MOV     R0, #0xdf
        MSR     CPSR_c, R0
        LDR     R13, =USR_Stack          

        .extern      Image_RO_Limit      	/* End of ROM code (=start of ROM data) */
        .extern      Image_RW_Base       	/* Base of RAM to initialise */
        .extern      Image_ZI_Base       	/* Base and limit of area */
        .extern      Image_ZI_Limit      	/* to zero initialise */

        ldr         r0, =Image_RO_Limit 	/* Get pointer to ROM data */
        ldr         r1, =Image_RW_Base  	/* and RAM copy */
        ldr         r3, =Image_ZI_Base  	/* Zero init base => top of initialised data */
        cmp         r0, r1                  /* Check that they are different */
        beq         NoRW
LoopRw: CMP     	r1, r3  				/* copy init data */
        LDRCC   	r2, [r0], #4
        STRCC   	r2, [r1], #4
        bcc     	LoopRw
NoRW:   LDR     	r1, =Image_ZI_Limit /* top of zero init segment */
        MOV     	r2, #0
LoopZI: CMP     	r3, r1  				/* zero init */
        STRCC   	r2, [r3], #4
        bcc     	LoopZI    
        			           
        MOV     R0, #0x1f
        MSR     CPSR_c, R0
        BL      main_code     

.global  dmult
.extern  dmult   
dmult:
	    STMFD   sp!, {r4-r5} 	
		UMULL   r4,r5,r0,r1
        STR     r5,[r2,#0]
        STR     r4,[r3,#0]
        LDMFD   sp!, {r4-r5}      			
        MOV     pc,lr


.global RunStartAddr
.extern RunStartAddr
RunStartAddr:
        STMFD   sp!, {r0-r12,lr}         
        MRS     r1, CPSR           
        BIC     r1, r1, #0x1F           
        ORR     r1, r1, #0x10   
        MSR     CPSR_c, r1 
        		
		#MOV     r1, #0x10
	    #MSR     CPSR_c,r1
	    
	    MOV     lr,pc 
		MOV     pc,r0
        LDMFD   sp!, {r0-r12,pc}^  
         			
.global RunAppStartAddr
.extern RunAppStartAddr
RunAppStartAddr:
        STMFD   sp!, {r0-r1,lr}          
	    MOV     lr,pc 
		MOV     pc,r0	
        LDMFD   sp!, {r0-r1,pc} 


.global ClearIDCache
.extern ClearIDCache
ClearIDCache:
        STMFD    sp!,{r0-r12,lr}
        @store cpsr then disable IRQ&FIQ
        MRS      r11,cpsr
        ORR      r12,r11,#0xC0
        MSR      cpsr_c,r12
        
        BL       MMU_DisableDCache
        BL       MMU_DisableICache
        BL       MMU_InvalidateICache
        MOV      r5,#0
loop_0:
        MOV      r4,#0
        MOV      r6,r5,LSL #26
loop_1:
        ORR      r0,r6,r4,LSL #5
        BL       MMU_CleanInvalidateDCacheIndex
        ADD      r4,r4,#1
        CMP      r4,#8
        BCC      loop_1
        ADD      r5,r5,#1
        CMP      r5,#0x40
        BCC      loop_0
        BL       MMU_EnableICache
        BL       MMU_EnableDCache  

	@restore cpsr enable IRQ&FIQ
        MSR    	 cpsr_c,r11    
        
        LDMFD    sp!, {r0-r12,pc} 

common_abort_handler:
	STMFD  SP!, {R2}	
	MOV      R0, SP
	@���浱ǰ��CPSR�ͷ����쳣��CPSR
	MRS      R2, CPSR
	MRS      R1, SPSR
	STMFD    R0!, {R1, R2}

	@����SYS Modem R13,R14
	MSR      CPSR_c, #0xDF @Disable I, Disable F
	STMFD    R0!, {R13,R14}
	STMFD    R0!, {R13,R14}
	MSR      CPSR, R2
	
	@����SVC Modem R13,R14
	MSR      CPSR_c, #0xD3 @Disable I, Disable F
	STMFD    R0!, {R13,R14}
	MSR      CPSR, R2
	
	@����IRQ Modem R13,R14
	MSR      CPSR_c, #0xD2 @Disable I, Disable F
	STMFD    R0!, {R13,R14}
	MSR      CPSR, R2
	
	@����FIQ Modem R13,R14
	MSR      CPSR_c, #0xD1 @Disable I, Disable F
	STMFD    R0!, {R13,R14}
	MSR      CPSR, R2
	
	MOV      SP, R0
	bl  _AbortHandler
        
.macro AbortHandler label, num
\label:
	STMFD  SP!, {R0 - R14}
	MOV    R2, #\num
	B      common_abort_handler
.endm

	
AbortHandler PrefetchAbortHandler,0
AbortHandler DataAbortHandler,1
AbortHandler UndefinedHandler,2
