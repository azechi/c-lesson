.globl _start
_start:
  ldr r13, =0x08000000  
  ldr r0, =0xdeadbeaf  
  bl print_hex
end:
  b end

/*
  putchar:
    arg r0: target charactor which is printed.
    used internal register: r1.
*/
putchar:
  stmdb r13!, {r1, r14}
  ldr r1, =0x101f1000
  str r0, [r1]
  ldmia r13!, {r1, r14}
  mov r15, r14

/*
  print_hex:
    arg r0: target 32bit data which is printed.
    used internal register: r1, r2
*/
print_hex:
  stmdb r13!, {r14}
  mov r1, r0
  mov r0, #0x30 // '0'
  bl putchar
  mov r0, #0x78 // 'x'
  bl putchar
  mov r2, #32 // loop counter
_loop:
  sub r2, r2, #4 
  lsr r0, r1, r2
  and r0, r0, #0xf
  cmp r0, #9
  ble _digit
  add r0, r0, #0x27
_digit:
  add r0, r0, #0x30
  bl putchar
  cmp r2, #0
  bne _loop
  ldmia r13!, {r14}
  mov r15, r14

