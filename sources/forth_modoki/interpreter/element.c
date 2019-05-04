#include <string.h>
#include <assert.h>
#include "clesson.h"


int element_equals(const struct Element e1, const struct Element e2) {
    if(e1.etype == e2.etype) {
        switch(e1.etype) {
            case ELEMENT_NUMBER:
                return (e1.u.number == e2.u.number);
            case ELEMENT_LITERAL_NAME:
                return (strcmp(e1.u.name, e2.u.name) == 0);
            default:
                return 0;
        }
    }
    return 0;
}

__attribute__((unused))
static void test_element_equals() {
    struct Element input_number_0 = {ELEMENT_NUMBER, {0}};
    struct Element input_number_0_0 = {ELEMENT_NUMBER, {0}};
    struct Element input_number_1 = {ELEMENT_NUMBER, {1}};
    struct Element input_literal_name_0 = {ELEMENT_LITERAL_NAME, .u.name = "a"};
    struct Element input_literal_name_0_0 = {ELEMENT_LITERAL_NAME, .u.name = "a"};
    struct Element input_literal_name_1 = {ELEMENT_LITERAL_NAME, .u.name = "b"};

    assert(element_equals(input_number_0, input_number_0_0));
    assert(!element_equals(input_number_0, input_number_1));

    assert(element_equals(input_literal_name_0, input_literal_name_0_0));
    assert(!element_equals(input_literal_name_0, input_literal_name_1));
}

#if 0
int main() { 
    test_element_equals();

    return 0;
}
#endif
