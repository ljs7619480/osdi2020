#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#define THREAD_SIZE                    4096
#define NR_TASKS                       64 

#define TASK_RUNNING                   0
#define TASK_ZOMBIE                    1

extern struct task_struct *current;
extern struct task_struct *task_pool[NR_TASKS];
extern int nr_tasks;

struct cpu_context {
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp;
    unsigned long lr;
    unsigned long sp;
};

struct user_page {
    unsigned long pa;//physical address
    unsigned long va;//virtual address
};

#define MAX_PROCESS_PAGES			16

struct mm_struct {
    unsigned long pgd;//ttbr0_el1
    int user_pages_count;
    struct user_page user_pages[MAX_PROCESS_PAGES];
};

struct task_struct {
    struct cpu_context cpu_context;
    long schedule_flag;
    long task_id;
    long state;
    long counter;
    // long priority;
    // long preempt_count;
    struct mm_struct mm;
};

long privilege_task_create(void(*func)());
void idle_task();
void schedule(void);
void timer_tick(void);
void context_switch(struct task_struct* next);
void cpu_switch_to(struct task_struct* prev, struct task_struct* next);
void task_preemption();
long do_get_taskid();
void do_exec(void(*func)());
void do_fork();
void do_exit();


#endif//_SCHEDULE_H