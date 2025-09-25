
[org 0x100]
section .data
    inputMsg db "val $"
    outputMsg db "val2 $"
    buffer db 5, 0       
    inputNum dw 0
    result dw 0

section .text
start:
  
    mov ah, 09h
    mov dx, inputMsg
    int 21h


    mov ah, 0Ah
    mov dx, buffer
    int 21h


    lea si, buffer + 2      
    xor cx, cx               
parse_loop:
    mov al, [si]
    cmp al, 13               
    je convert_done
    sub al, '0'              
    mov bl, 10
    mul bl                  
    add cx, ax              
    jmp parse_loop

convert_done:
    mov inputNum, cx

   
    mov ax, inputNum
    shl ax, 1
    mov result, ax

   
    mov ax, result
    call PrintNumber

 
    mov ah, 4Ch
    int 21h

PrintNumber:
    ; AX'te sayı var
    push ax
    mov cx, 0
    mov bx, 10

.next_digit:
    xor dx, dx
    div bx             
    push dx            
    inc cx           
    test ax, ax
    jnz .next_digit

 
    mov ah, 09h
    mov dx, outputMsg
    int 21h

.print_loop:
    pop dx
    add dl, '0'
    mov ah, 02h
    int 21h
    loop .print_loop

    pop ax
    ret
