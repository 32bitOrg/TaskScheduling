# TaskScheduling

This project presents the design and implementation of a round-robin scheduler on the STM32F401CCU6 microcontroller.

The scheduler operates by allocating time slices to each task in equal portions and in a circular order, handling all the tasks without priority.
The project includes four user tasks, each of which has its own stack to create local variables when it runs on the CPU. 
The context or state of the task is saved in the task’s private stack when the scheduler decides to remove a task from the CPU.

The scheduler is implemented using the SysTick Timer and the PendSV handlers.

https://youtu.be/fv2mA673GRs?si=mkrsn2FLIzauexa7
