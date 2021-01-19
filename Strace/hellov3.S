/* 
Ivan Chowdhury
ECE 357: Computer Operating Systems
Fall 2018
Hello World in Assembly v2 (Using x86-64)
*/
 
.global _start

.text

_start:

        mov     $1, %rax             # write(
        mov     $1, %rdi             # STDOUT
        mov     $999, %rsi           # Invalid Parameter for write syscall
        mov     $14, %rdx            # Message length (14 characters)
        syscall                      # Execute syscall

        mov     $60, %rax            # _exit(
        mov     $5, %rdi             # Exit code 5
        syscall                      # Execute syscall
msg:
        .ascii  "Hello, world.\n"    # Message