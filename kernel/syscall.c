#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/mem.h>
#include <kernel/cpu.h>
#include <kernel/syscall.h>
#include <kernel/trap.h>
#include <inc/stdio.h>

extern int sys_fork();
extern void sched_yield();
extern void sys_kill(int pid);
extern int32_t sys_get_num_used_page();
extern int32_t sys_get_num_free_page();
extern unsigned long sys_get_ticks();
extern void sys_cls();
extern void sys_settextcolor();

void do_puts(char *str, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; i++)
	{
		k_putch(str[i]);
	}
}

int32_t do_getc()
{
	return k_getc();
}

int32_t do_syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t retVal = -1;

	switch (syscallno)
	{
		case SYS_fork:
			/* TODO: Lab 5
			* You can reference kernel/task.c, kernel/task.h */
			retVal = sys_fork();
			break;

		case SYS_getc:
			retVal = do_getc();
			break;

		case SYS_puts:
			do_puts((char*)a1, a2);
			retVal = 0;
			break;

		case SYS_getpid:
			/* TODO: Lab 5
			* Get current task's pid */
			retVal = thiscpu->cpu_task->task_id;
			break;
		case SYS_getcid:
			retVal = thiscpu->cpu_id;
			break;

		case SYS_sleep:
			/* TODO: Lab 5
			* Yield this task
			* You can reference kernel/sched.c for yielding the task */
			thiscpu->cpu_task->remind_ticks = a1;
			thiscpu->cpu_task->state = TASK_SLEEP;
			sched_yield();
			break;

		case SYS_kill_self:
			/* TODO: Lab 5
			* Kill specific task
			* You can reference kernel/task.c, kernel/task.h*/
			sys_kill(thiscpu->cpu_task->task_id);
			break;

		case SYS_get_num_free_page:
			/* TODO: Lab 5
			* You can reference kernel/mem.c*/
			retVal = sys_get_num_free_page();
			break;

		case SYS_get_num_used_page:
			/* TODO: Lab 5
			* You can reference kernel/mem.c */
			retVal = sys_get_num_used_page();
			break;

		case SYS_get_ticks:
			/* TODO: Lab 5
			* You can reference kernel/timer.c*/
			retVal = sys_get_ticks();
			break;

		case SYS_settextcolor:
			/* TODO: Lab 5
			*You can reference kernel/screen.c*/
			sys_settextcolor(a1, a2);
			break;

		case SYS_cls:
			/* TODO: Lab 5
			* You can reference kernel/screen.c*/
			sys_cls();
			break;

	}
	return retVal;
}

static void syscall_handler(struct Trapframe *tf)
{
	/* TODO: Lab5
   * call do_syscall
   * Please remember to fill in the return value
   * HINT: You have to know where to put the return value
   */
   tf->tf_regs.reg_eax = do_syscall(tf->tf_regs.reg_eax,
   				    tf->tf_regs.reg_edx,
   				    tf->tf_regs.reg_ecx,
   				    tf->tf_regs.reg_ebx,
   				    tf->tf_regs.reg_edi,
   				    tf->tf_regs.reg_esi);

}

void syscall_init()
{
  /* TODO: Lab5
   * Please set gate of system call into IDT
   * You can leverage the API register_handler in kernel/trap.c
   */
   extern void syscall_entry();
   register_handler(T_SYSCALL, syscall_handler, syscall_entry, 1, 3);

}

