/*
 arm-none-eabi-as print_hex.s -o print_hex.o
 arm-none-eabi-ld print_hex.o -Ttext 0x00010000 -o print_hex.elf
 arm-none-eabi-objcopy print_hex.elf -O binary print_hex.bin
 qemu-system-arm -M versatilepb -m 128M -nographic -kernel print_hex.bin -serial mon:stdio
*/
.globl _start
_start:
    ldr r0, =0x101f1000
    ldr r1, =0xdeadbeaf
    
    mov r2, #0x30 // '0'
    str r2, [r0]
    mov r2, #0x78 // 'x'    
    str r2, [r0]

    mov r2, #32 // loop counter
    b print_hex

print_hex:    
    sub r2, r2, #4 
    lsr r3, r1, r2
    and r3, r3, #0xf
    cmp r3, #9
    ble digit
    add r3, r3, #0x27

digit:
    add r3, r3, #0x30
    str r3, [r0]
    cmp r2, #0
    bne print_hex

end:
    b end

