/* 
Ivan Chowdhury
ECE 357: Computer Operating Systems
Fall 2018
Hello World in Assembly (Using x86-64)
*/
 
.global _start

.text

_start:

        mov     $1, %rax             # write() syscall
        mov     $1, %rdi             # STDOUT
        mov     $msg, %rsi           # Message string
        mov     $14, %rdx            # Message length (14 characters)
        syscall                      # Execute syscall

msg:
        .ascii  "Hello, world.\n"    # Message