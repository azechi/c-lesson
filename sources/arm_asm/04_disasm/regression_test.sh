find test/test_input/ -type f | sed "s|^.*/||" | xargs -L1 sh -c 'echo $0;./a.out test/test_input/$0 | diff -  test/test_expect/${0%.bin}.txt; echo $?'

