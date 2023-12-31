/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "led.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif



void task1_handler(void);// This is task 1
void task2_handler(void);// This is task 2
void task3_handler(void);// This is task 3
void task4_handler(void);// This is task 4 of the application

void init_systick_timer(uint32_t tick_hz);
__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_of_stack);
void init_tasks_stack(void);
void enable_processor_faults(void);
__attribute__((naked)) void switch_sp_to_psp(void);
uint32_t get_psp_value(void);
void save_psp_value(uint32_t current_psp_value);
void idle_task(void);
void update_global_tick_count(void);



/* This section is not required after introducing a structure for each task
// array to store the psp's of the tasks
uint32_t psp_of_tasks[MAX_TASKS]={T1_STACK_START,T2_STACK_START,T3_STACK_START,T4_STACK_START};

// array to store the different addresses of task handlers
uint32_t task_handlers[MAX_TASKS];
*/


void task_delay(uint32_t tick_count);
void schedule(void);


uint32_t current_task = 1; // global variable
//current task 1 means task1 is running

uint32_t g_tick_count = 0;

// introducing blocking state for tasks
typedef struct{
	uint32_t psp_value;
	uint32_t block_count;
	uint8_t current_state;
	void (*task_handler)(void);
}TCB_t;

TCB_t user_tasks[MAX_TASKS];




void idle_task(void){
	while(1);
}


void task1_handler(void)
{
	while(1){
		led_on(LED_GREEN);
		task_delay(1000);
		led_off(LED_GREEN);
		task_delay(1000);

	}
}

void task2_handler(void)
{
	while(1){
		led_on(LED_ORANGE);
		task_delay(500);
		led_off(LED_ORANGE);
		task_delay(500);
	}
}

void task3_handler(void)
{
	while(1){
		led_on(LED_BLUE);
		task_delay(250);
		led_off(LED_BLUE);
		task_delay(250);
	}
}

void task4_handler(void)
{
	while(1){
		led_on(LED_RED);
		task_delay(125);
		led_off(LED_RED);
		task_delay(125);
	}
}

void init_systick_timer(uint32_t tick_hz)
{
	// systick - 24bit down counter
	//syticjk eception is triggered inly after the value is reloaded into the SVR from the CVr(current value register) after the svr value reaches 0. Hence we load count value -1 into the svr so that it takes the required no. of cycles (otherwise it would rake count +1 cycles)
	uint32_t *pSRVR = (uint32_t*)0xE000E014;// a pointer variable pointing to the reload value location
	uint32_t *pSCSR = (uint32_t*)0xE000E010;// pointer for control and status register pointing to the systick control and status register

	uint32_t count_value = (SYSTICK_TIM_CLK/tick_hz)-1; // also known as reload value
	// only 24 bits are valid in this 32 bit register, rest should remain unaffected

	//Clear the value of SVR
	*pSRVR &= ~(0x00FFFFFFFF);// since only 24 bits are valid (reserved ones not affected)

	//load the value into SVR
	*pSRVR |= count_value;

	//do some settings
	// systick control and status register
	*pSCSR |= (1<<1); //Enable Systick exception request
	*pSCSR |= (1<<2); //Indicates the clock source, processor clock source

	//enable the systick
	*pSCSR |= (1<<0); //enables the counter
}

__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_of_stack)
{
	__asm volatile("MSR MSP,%0": :  "r" (sched_top_of_stack)  :  );
	__asm  volatile("BX LR");// LR has the return address, to return from function call
}




void init_tasks_stack(void)
{
	// initially all tasks in Running State
	user_tasks[0].current_state =TASK_READY_STATE;// task 0 becomes the idle task which will always be in running state
	user_tasks[1].current_state =TASK_READY_STATE;
	user_tasks[2].current_state =TASK_READY_STATE;
	user_tasks[3].current_state =TASK_READY_STATE;
	user_tasks[4].current_state =TASK_READY_STATE;

	user_tasks[0].psp_value = IDLE_STACK_START;
	user_tasks[1].psp_value = T1_STACK_START;
	user_tasks[2].psp_value = T2_STACK_START;
	user_tasks[3].psp_value = T3_STACK_START;
	user_tasks[4].psp_value = T4_STACK_START;

	user_tasks[0].task_handler = idle_task;
	user_tasks[1].task_handler = task1_handler;
	user_tasks[2].task_handler = task2_handler;
	user_tasks[3].task_handler = task3_handler;
	user_tasks[4].task_handler = task4_handler;

/*
	uint32_t *pPSP;//pointer to access data being stored in the stack memory

	for(int i=0;i<MAX_TASKS;i++){

		pPSP = (uint32_t*) psp_of_tasks[i];//type casting

		//stack is full descending and hence we decrement and then store the value
		pPSP--;
		*pPSP = DUMMY_XPSR; //0x01000000

		pPSP--;//PC
		*pPSP = task_handlers[i];// here have to store the address of task handlers
		// this address should be odd since the lsb represents the t bit

		pPSP--;//LR
		*pPSP = 0xFFFFFFFD;

		for (int j=0;j<13;j++){
			pPSP--;
			*pPSP=0;
		}
		// preserving the psp value
		psp_of_tasks[i]=(uint32_t)pPSP;
	}
*/

	//modified version of the above commented code

	uint32_t *pPSP;//pointer to access data being stored in the stack memory

	for(int i=0;i<MAX_TASKS;i++){

		pPSP = (uint32_t*) user_tasks[i].psp_value;//type casting

		//stack is full descending and hence we decrement and then store the value
		pPSP--;
		*pPSP = DUMMY_XPSR; //0x01000000

		pPSP--;//PC
		*pPSP = (uint32_t) user_tasks[i].task_handler;// here have to store the address of task handlers
		// this address should be odd since the lsb represents the t bit

		pPSP--;//LR
		*pPSP = 0xFFFFFFFD;

		for (int j=0;j<13;j++){
			pPSP--;
			*pPSP=0;
		}
		// preserving the psp value
		user_tasks[i].psp_value=(uint32_t)pPSP;
	}
}

void enable_processor_faults(void){
	uint32_t *pSHCSR = (uint32_t*)0xE000ED24;

	*pSHCSR |= (1<<16); // mem manage
	*pSHCSR |= (1<<17);// bus fault
	*pSHCSR |= (1<<18);// usage fault

}




//helper function
uint32_t get_psp_value(void){
	return user_tasks[current_task].psp_value;

}

void save_psp_value(uint32_t current_psp_value){
	user_tasks[current_task].psp_value=current_psp_value;

}

void update_next_task(void){

	int state = TASK_BLOCKED_STATE;

	for(int i=0;i<MAX_TASKS;i++){
		current_task++;
		current_task %= MAX_TASKS;
		state = user_tasks[current_task].current_state;
		if((state==TASK_READY_STATE) && (current_task != 0)){
			break;
		}

	}
	if (state!=TASK_READY_STATE)
		current_task = 0;
}

__attribute__((naked)) void switch_sp_to_psp(void){
	//1. initialize the PSP with Task1 stack start address

	// get the value of psp of current_task
	//here since we use Bl, LR's initial value is corrupted which has the address of the main
	//hence the LR has to be pushed/saved before it is modified
	__asm volatile ("PUSH {LR}");// preserve LR which connects back to main()

	__asm volatile ("BL get_psp_value");// return value is stored in R0
	__asm volatile ("MSR PSP,R0");// initialize psp

	__asm volatile ("POP {LR}");//pops back LR value

	//till here everything is using MSP as stack pointer since we have not yet executed a switch

	//2. change SP to PSP using CONTROL register
	// the CONTROL register is a spl register and we have to use MSR instruction and hence this function is naked
	// in the CONTROL register (2nd bit should be 1 which is SPSEl (defines the currently active stack pointer, 0=MSP and 1=PSP)
	__asm volatile ("MOV R0,#0x02");//enables second bit
	__asm volatile ("MSR CONTROL,R0");

	//go back to main
	__asm volatile ("BX LR");// LR value copied to PC that connects us back to the main()

}
// Processor always uses MSP in Handler mode
// compiler does not add Prologue ((or preamble) Save registers and return address; transfer parameters) or epilogue ((or postamble) Restore registers; transfer returned value; return) sequences for functions declared naked.
//The prologue and epilogue associated with each procedure are “overhead” that is necessary but does not do user computation.
// epilogues is not added and hence we have to manually write instructions to make exception exit



void schedule(void){
	//pend the pendSV exception
	uint32_t *pICSR = (uint32_t*)0xE000ED04;
	*pICSR |= (1<<28);
}


void task_delay(uint32_t tick_count){
	// to be careful with global variable access
	// disable interrupt
	INTERRUPT_DISABLE();

	if(current_task){
		// since we have to block only user task, this condition will not execute for current_task=0 (idle task)
		user_tasks[current_task].block_count = g_tick_count + tick_count;
		user_tasks[current_task].current_state = TASK_BLOCKED_STATE;
		schedule();// to trigger the pendSV
	}

	//enable interrupt
	INTERRUPT_ENABLE();
}

__attribute__((naked)) void PendSV_Handler(void){
	//using in-line assembly code again


		// Save the context of current task

		//1. Get current running task's PSP value
		__asm volatile ("MRS R0,PSP");
		//2. Using that PSP value store SF2(R4 to R11)
		//we can't use push operations from this handler as it would change the MSP
		// using store multiple instructions - STMDB (decrement and then store)
		__asm volatile ("STMDB R0!,{R4-R11}");

		__asm volatile ("PUSH {LR}");

		//3. Save the current value of PSP
		__asm volatile("BL save_psp_value");// when this function is called R0's value is sent as parameter to the function




		// Retrieve the context of next task

		//1. Decide next task to run
		__asm volatile("BL update_next_task");
		//2. get its past PSP value
		__asm volatile ("BL get_psp_value");// written value in R0
		//3. Using that PSP value retrieve SF2(R4 to R11), using LOAD instruction since data moves from memory to register
		// LDMIA - load multiple registers and increment after (default)
		__asm volatile("LDMIA R0!,{R4-R11}");
		//4. Update PSP by saving R0 to psp and exit
		__asm volatile ("MSR PSP,R0");

		__asm volatile ("POP {LR}");

		//exit (LR copied to PC)
		__asm  volatile ("BX LR");

}

void update_global_tick_count(void){
	g_tick_count++;
}

void unblock_tasks(void){
	//not checking for idle task i=0
	for(int i=1;i<MAX_TASKS;i++){
		if(user_tasks[i].current_state != TASK_READY_STATE){
			if(user_tasks[i].block_count == g_tick_count){
				user_tasks[i].current_state = TASK_READY_STATE;
			}
		}
	}
}

void SysTick_Handler(void)
{
	uint32_t *pICSR = (uint32_t*)0xE000ED04;

	update_global_tick_count();

	unblock_tasks();

	//pend the pendsv exception
	*pICSR |= (1<<28);
}

//Fault Handler Codes
void HardFault_Handler(void){
	printf("Exception :  HardFault\n");
	while(1);
}

void MemManage_Handler(void){
	printf("Exception :  MemManage\n");
	while(1);
}

void BusFault_Handler(void){
	printf("Exception :  BusFault\n");
	while(1);
}


int main(void){

	// enabling all available faults and fault handlers to trace any faults that could take place
	enable_processor_faults();

	init_scheduler_stack(SCHED_STACK_START);


/*  This section is not required after introducing a structure for each task
	task_handlers[0]=(uint32_t)task1_handler;
	task_handlers[1]=(uint32_t)task2_handler;
	task_handlers[2]=(uint32_t)task3_handler;
	task_handlers[3]=(uint32_t)task4_handler;
	// these are now assigned in the init_tasks_stack() itself
*/


	init_tasks_stack();

	led_init_all();


	init_systick_timer(TICK_HZ);


	// till here the code is using MSP as stack pointer
	// we can call the task1_handler here
	// hence the stack pointer has to be changed to PSP, since tasks should run in the Thread mode
	switch_sp_to_psp();

	//first task getting launched from the main we can also use an svc instruction to launch
	task1_handler();


	for(;;);
}
