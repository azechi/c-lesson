#include <stdio.h>
#include <assert.h>

static const char* const input = "123 456  1203";


int main() {
    int answer1 = 0;
    int answer2 = 0;
    int answer3 = 0;

    // write something here.
    int* ans[] = {&answer1, &answer2, &answer3};
    int p = 0;
 
    const char* i = input;
    int acm = 0;
    
    while(*i) {
        if(*i == ' '){
            *ans[p++] = acm;
            acm = 0;

            while(*++i == ' ')
                ;
            
            continue;
        }

        acm = acm * 10 + *i - '0';
        
        i++;
    }

    *ans[p] = acm;

    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);
    assert(answer3 == 1203);

    return 1;
}
