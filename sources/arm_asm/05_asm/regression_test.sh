find test/test_input/ -type f | sed "s|^.*/||" | xargs -L1 bash -c 'echo $0; ./a.out test/test_input/$0 | od -Ax -w4 -tx4 | diff -  <(cat test/test_expect/${0%.ks}.bin | od -Ax -w4 -tx4); echo $?'
