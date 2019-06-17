  ldr r13, =0x08000000
  ldr r0, =0xdeadbeaf
  bl print_hex
  mov r0, r15
  bl print_hex
  bl r14
  
end:
  b end

r14:
  mov r0, r14
  bl print_hex
  mov r15, r14

putchar:
  stmdb r13!, {r1, r14}
  ldr r1, =0x101f1000
  str r0, [r1]
  ldmia r13!, {r1, r14}
  mov r15, r14

print_hex:
  stmdb r13!, {r1, r2, r14}
  mov r1, r0
  mov r0, #0x30
  bl putchar
  mov r0, #0x78
  bl putchar
  mov r2, #0x20
_loop:
  sub r2, r2, #0x4 
  lsr r0, r1, r2
  and r0, r0, #0x0f
  cmp r0, #0x09
  ble _digit
  add r0, r0, #0x27
_digit:
  add r0, r0, #0x30
  bl putchar
  cmp r2, #0x00
  bne _loop
  mov r0, #0x0a
  bl putchar
  ldmia r13!, {r1, r2, r14}
  mov r15, r14

