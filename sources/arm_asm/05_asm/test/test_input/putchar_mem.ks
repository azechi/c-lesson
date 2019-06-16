  ldr r13, =0x08000000
  ldr r0, =msg1
  bl print
  ldr r0, =msg2
  bl print
end:
  b end
putchar:
  stmdb r13!, {r1, r14}
  ldr r1, =0x101f1000
  str r0, [r1]
  ldmia r13!, {r1, r14}
  mov r15, r14
print:
  stmdb r13!, {r1, r14}
  mov r1, r0
  ldrb r0, [r1]
_loop:
  bl putchar
  add r1, r1, #0x01
  ldrb r0, [r1]
  cmp r0, #0x00
  bne _loop
  ldmia r13!, {r1, r14}
  mov r15, r14
msg1:
  .raw "First text.\n"
msg2:
  .raw "Second text!\n"
