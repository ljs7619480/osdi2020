#include "schedule.h"

#include "exce.h"
#include "mm.h"
#include "printf.h"
#include "uart.h"
#include "sysregs.h"
#include "lib/string.h"

/*
    get ksp base vs get ksp top
    task manage seperate task_struct from ksp
    current -> tpidr_el1
*/

static struct task_struct init_task;
struct task_struct *task_pool[NR_TASKS]={&init_task, 0};   //task array
struct task_struct *current = &init_task;      //currently executing task


void idle_task(){
    while(1){
        #ifdef __DEBUG
        int cnt = 100000000;
        while(cnt--);
        #endif//DEBUG
        schedule();
  }
}

void schedule(void)
{
    #ifdef __DEBUG
    printf("[schedule] Task_id: %d\n", current->task_id);
    #endif//__DEBUG
    long idx = current->task_id;
    struct task_struct *next = task_pool[(++idx)%NR_TASKS];
    while(!(next && next->state==TASK_RUNNING))
        next = task_pool[(++idx)%NR_TASKS];
    context_switch(next);
}

void _context_switch()
{
    asm ("mov x10, #0");
    asm ("add x8, x0, x10");
    asm ("mov x9, sp");
    asm ("stp x19, x20, [x8], #16");
    asm ("stp x21, x22, [x8], #16");
    asm ("stp x23, x24, [x8], #16");
    asm ("stp x25, x26, [x8], #16");
    asm ("stp x27, x28, [x8], #16");
    asm ("stp x29, x30, [x8], #16"); //fp,lr
    asm ("str x9, [x8]");//sp
    asm ("add x8, x1, x10");
    asm ("ldp x19, x20, [x8], #16");
    asm ("ldp x21, x22, [x8], #16");
    asm ("ldp x23, x24, [x8], #16");
    asm ("ldp x25, x26, [x8], #16");
    asm ("ldp x27, x28, [x8], #16");
    asm ("ldp x29, x30, [x8], #16");
    asm ("ldr x9, [x8]");
    asm ("mov sp, x9");
	// asm ("msr tpidr_el1, x1");     // store thread identifying information, for OS management purposes.
    return;                        // retruen to $lr($x30)
}

void context_switch(struct task_struct *next) 
{
    #ifdef __DEBUG
    printf("[context_switch] Task_id: %d\n", current->task_id);
    #endif//__DEBUG
    if (current == next) 
        return;
    struct task_struct *prev = current;
    current = next;
    _context_switch(prev, next);
}

long assign_task_id()
{   
    for(long id=0; id<NR_TASKS; id++){
        if(task_pool[id]) // non Null ptr, if assigned.
            continue;
        return id;
    }
    uart_puts("Task pool had fulled!\n");
    return -1;
}

//Our sp offset is base on pTask. It only works on fix stack location.
long privilege_task_create(void(*func)())
{
    struct task_struct *pTask;
    long task_id;
    if( (task_id = assign_task_id()) < 0 ){
        return task_id;
        uart_puts("Fail to assign task id\n");
    }
    pTask = (struct task_struct *)get_kstack_base(task_id);
    pTask->state = TASK_RUNNING;
    pTask->cpu_context.sp = get_kstack_base(task_id) + THREAD_SIZE;
    pTask->cpu_context.lr = (unsigned long)func;
    pTask->counter = 4;
    pTask->schedule_flag = 0;
    pTask->task_id = task_id;
    task_pool[task_id] = pTask;
    #ifdef __DEBUG
    printf("[pcreate] Task_id: %d\tpTask: 0x%X\tksp: 0x%X\n", task_id, pTask, pTask->cpu_context.sp);
    #endif//__DEBUG
    return task_id;
}

long do_get_taskid(){
    long task_id = current->task_id;
    #ifdef __DEBUG
        printf("[get Task_id] Task_id: %d\n", task_id);
    #endif
    return task_id; 
}

void do_exec(void(*func)())
{
    unsigned long usp = get_ustack_base(current->task_id) + THREAD_SIZE;
    #ifdef __DEBUG
    printf("[exec] Task_id: %d\tusp: 0x%X\n", current->task_id, usp);
    #endif//__DEBUG
    asm volatile("msr sp_el0, %0"::"r"(usp):);
    asm volatile("msr spsr_el1, %0"::"r"(SPSR_EL1_VALUE):);
    asm volatile("msr elr_el1, %0"::"r"(func):);
    asm volatile("eret");
}

void do_fork(struct trapframe *tf)
{
    #ifdef __DEBUG
    printf("[fork start] <P>Task_id: %d\n", current->task_id);
    #endif//__DEBUG
    struct task_struct *pTask;
    long task_id = assign_task_id();
    pTask = (struct task_struct *)get_kstack_base(task_id);
    task_pool[task_id] = pTask;

    memcpy((void*)get_kstack_base(task_id), (void*)get_kstack_base(current->task_id), THREAD_SIZE);//kernel stack
    memcpy((void*)get_ustack_base(task_id), (void*)get_ustack_base(current->task_id), THREAD_SIZE);//user stack

    pTask->task_id = task_id;
    // set ksp
    unsigned long tf_offset = (unsigned long)tf - (unsigned long)current;
    pTask->cpu_context.sp = (unsigned long)pTask + tf_offset;
    // set usp
    struct trapframe *utf = (struct trapframe *)pTask->cpu_context.sp;
    unsigned long usp_offset = tf->sp_el0 - (unsigned long)current;
    utf->sp_el0 = (unsigned long)pTask + usp_offset;
    // set fp
    // Note: complier may use fp to access function local variable. not sp
    unsigned long fp_offset = (unsigned long)tf->fp - (unsigned long)current;
    utf->fp = (unsigned long)pTask + fp_offset;
    // set return
    tf->Xn[0] = task_id;
    utf->Xn[0] = 0;
    // need not set cpu context, kernel exit will overwrite it.
    pTask->cpu_context.lr = (unsigned long)ret_fork_child;
    #ifdef __DEBUG
    printf("[fork end] <c>Task_id: %d\tpTask: 0x%X\tksp: 0x%X\tusp: 0x%X\n", task_id, pTask, pTask->cpu_context.sp, utf->sp_el0);
    #endif//__DEBUG
}

void do_exit(int status)
{
    #ifdef __DEBUG
    printf("[exit] Task_id: %d\n", current->task_id);
    #endif//__DEBUG
    current->state = TASK_ZOMBIE;
    current->schedule_flag = 1;
    // should call schedule later by task preemption
}

void timer_tick()
{
    enable_irq();
    if(current->counter > 0){
        current->counter--;
        printf(">>>>Task_id:%d  Counter value: %d\n",
                 current->task_id, current->counter);
    }else
        current->schedule_flag=1;
    disable_irq();
}

void task_preemption()
{
    #ifdef __DEBUG
    printf("[task_preemption] Task_id: %d\n",current->task_id);
    #endif//__DEBUG
    if(current->schedule_flag)
        schedule();
}