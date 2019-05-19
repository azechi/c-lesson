#include "def.h"
#include "call_stack.h"


typedef enum CallStackItemType_ {
    CALLSTACKITEM_UNKNOWN,
    CALLSTACKITEM_VARIABLE,
    CALLSTACKITEM_CONTINUATION
} CallStackItemType;

typedef struct CallStackItem_ {
    enum CallStackItemType_ ctype;
    union {
        struct Continuation_ continuation;
        Element variable;
    } u;
} CallStackItem;


#define STACK_SIZE 1024


static CallStackItem stack[STACK_SIZE];
static int sp = 0; /* stack pointer */


static void stack_push(const CallStackItem *item);
static int try_stack_pop(CallStackItem *out_item);


void eval_exec_array(Execute execute, const ElementArray *exec_array) {
    CallStackItem item = {
        .ctype = CALLSTACKITEM_CONTINUATION,
        .u.continuation = {
            .exec_array = exec_array,
            .pc = 0
        }
    };
    stack_push(&item);

    while(try_stack_pop(&item)) {
        if(item.ctype == CALLSTACKITEM_CONTINUATION) {
            execute(&item.u.continuation);
        }
    }
}

void call_exec_array(const ElementArray *exec_array, const Continuation *co) {
    CallStackItem item = {
        .ctype = CALLSTACKITEM_CONTINUATION,
        .u.continuation = *co
    };
    item.u.continuation.pc += 1;
    stack_push(&item);

    item = (CallStackItem){
        .ctype = CALLSTACKITEM_CONTINUATION,
        .u.continuation = {
            .exec_array = exec_array,
            .pc = 0
        }
    };
    stack_push(&item);
}


void local_variable_get(int offset, Element *out_el) {
    int index = (sp - 1) - offset;
    
    int i;
    for(i = (sp - 1); i >= index; i--) {
        if((stack + i)->ctype != CALLSTACKITEM_VARIABLE) {
            assert_fail("VARIABLE NOT FOUND");
        }
    }

    *out_el = (stack + index)->u.variable;
}

void local_variable_pop() {
    if((stack + sp - 1)->ctype != CALLSTACKITEM_VARIABLE) {
        assert_fail("VARIABLE NOT FOUND");
    }

    sp--;
}

void local_variable_push(const Element *el) {
    CallStackItem item = {
        .ctype = CALLSTACKITEM_VARIABLE,
        .u.variable = *el
    };
    stack_push(&item);
}


/* */

static void stack_push(const CallStackItem *item) {
    assert(sp < STACK_SIZE);

    if(sp < STACK_SIZE) {
        stack[sp++] = *item;
    }
}

static int try_stack_pop(CallStackItem *out_item) {
    if(sp > 0) {
        *out_item = stack[--sp];
        return 1;
    }
    return 0;
}

