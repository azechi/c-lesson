.globl _start
_start:
  ldr r13, =0x08000000  //10000
  ldr r0, =0xdeadbeaf   //10004
  bl print_hex          //10008
  mov r0, r15           //1000c r15=0x00010014
  bl print_hex          //10010
  bl r14                //10014 r14=0x00010018
  
end:
  b end                 //10018

r14:
  mov r0, r14
  bl print_hex
  mov r15, r14

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
  stmdb r13!, {r1, r2, r14}
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
  mov r0, #0x0a // LF
  bl putchar
  ldmia r13!, {r1, r2, r14}
  mov r15, r14

