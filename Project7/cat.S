.global _start

.bss
    buffer: .skip 4096 #creates buffer

.text

_start:
mov $0, %rax
mov $0, %rdi
mov $buffer, %rsi
mov $4096, %rdx
syscall

mov $1, %rax
mov $1, %rdi
mov $buffer, %rsi
mov $4096, %rdx
syscall

mov $60, %rax
mov $0, %rdi
syscall