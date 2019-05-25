.globl _start
_start:

  ldr r13, =0x08000000
  ldr r0, =msg1
  bl print
  ldr r0, =msg2
  bl print
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
  print:
    arg r0: Address of target string, must end by \0.
    used internal register: r1
*/
print:
  stmdb r13!, {r1, r14}
  mov r1, r0
  ldrb r0, [r1] // load charactor
_loop:
  bl putchar
  add r1, r1, #1
  ldrb r0, [r1]
  cmp r0, #0
  bne _loop
  ldmia r13!, {r1, r14}
  mov r15, r14

msg1: .asciz "First text.\n"
msg2: .asciz "Second text!\n"
