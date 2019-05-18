#include "util.h"
#include "parser.h"
#include "element.h"
#include "auto_element_array.h"
#include "dict.h"


static Element to_element(const Token *);
static void emit_number(AutoElementArray *, int);
static void emit_executable_name(AutoElementArray *, char*);
static void emit_primitive(AutoElementArray *, Operator);


Element compile_exec_array(int ch, int *out_ch) {
    AutoElementArray elements = {0};
    Token token = {0};

    auto_element_array_init(&elements);

    do {
        ch = parse_one(ch, &token);
        switch(token.ltype) {
            case LEX_EXECUTABLE_NAME:
                {
                    Element el = {0};
                    if(compile_dict_get(token.u.name, &el)) {
                        el.u.compile_func(&elements);
                        break;
                    }
                }
                /* fallthrough  */
            case LEX_NUMBER:
            case LEX_LITERAL_NAME:
                {
                    Element el = to_element(&token);
                    auto_element_array_add_element(&elements, &el);
                }
                break;
            case LEX_CLOSE_CURLY:
            case LEX_SPACE:
                break;
            case LEX_OPEN_CURLY:
                {
                    Element el = compile_exec_array(ch, &ch);
                    auto_element_array_add_element(&elements, &el);
                }
                break;
            case LEX_END_OF_FILE:
            default:
                assert_fail("NOT IMPLEMENTED");
                break;
        }
    } while(token.ltype != LEX_CLOSE_CURLY);

    *out_ch = ch;

    auto_element_array_trim_to_size(&elements);
    return (Element){ELEMENT_EXEC_ARRAY, .u.exec_array = elements.var_array};
}


static void ifelse_compile(AutoElementArray *emitter) {
    emit_number(emitter, 3);
    emit_number(emitter, 2);
    emit_executable_name(emitter, "roll");
    emit_number(emitter, 5);
    emit_primitive(emitter, OP_JMP_NOT_IF);
    emit_executable_name(emitter, "pop");
    emit_primitive(emitter, OP_EXEC);
    emit_number(emitter, 4);
    emit_primitive(emitter, OP_JMP);
    emit_executable_name(emitter, "exch");
    emit_executable_name(emitter, "pop");
    emit_primitive(emitter, OP_EXEC);
}

static void if_compile(AutoElementArray *emitter) {
    emit_executable_name(emitter, "exch");
    emit_number(emitter, 4);
    emit_primitive(emitter, OP_JMP_NOT_IF);
    emit_primitive(emitter, OP_EXEC);
    emit_number(emitter, 2);
    emit_primitive(emitter, OP_JMP);
    emit_executable_name(emitter, "pop");
}

static void exec_compile(AutoElementArray *emitter) {
    emit_primitive(emitter, OP_EXEC);
}

static void jmp_compile(AutoElementArray *emitter) {
    emit_primitive(emitter, OP_JMP);
}

static void jmp_not_if_compile(AutoElementArray *emitter) {
    emit_primitive(emitter, OP_JMP_NOT_IF);
}

static void store_compile(AutoElementArray *emitter) {
    emit_primitive(emitter, OP_STORE);
}

static void load_compile(AutoElementArray *emitter) {
    emit_primitive(emitter, OP_LOAD);
}


void register_primitive() {
    compile_dict_put_compile_func("ifelse", ifelse_compile);
    compile_dict_put_compile_func("if", if_compile);
    compile_dict_put_compile_func("exec", exec_compile);
    compile_dict_put_compile_func("jmp", jmp_compile);
    compile_dict_put_compile_func("jmp_not_if", jmp_not_if_compile);
    compile_dict_put_compile_func("store", store_compile);
    compile_dict_put_compile_func("load", load_compile);
}


static void emit_number(AutoElementArray *emitter, int number) {
    Element el = {ELEMENT_NUMBER, .u.number = number};
    auto_element_array_add_element(emitter, &el);
}

static void emit_executable_name(AutoElementArray *emitter, char *name) {
    Element el = {ELEMENT_EXECUTABLE_NAME, .u.name = name};
    auto_element_array_add_element(emitter, &el);
}

static void emit_primitive(AutoElementArray *emitter, Operator op) {
    Element el = {ELEMENT_PRIMITIVE, .u.operator = op};
    auto_element_array_add_element(emitter, &el);
}

static Element to_element(const Token *token) {
    Element el = {0};

    switch(token->ltype) {
        case LEX_NUMBER:
            el.etype = ELEMENT_NUMBER;
            el.u.number = token->u.number;
            break;
        case LEX_EXECUTABLE_NAME:
            el.etype = ELEMENT_EXECUTABLE_NAME;
            el.u.name = token->u.name;
            break;
        case LEX_LITERAL_NAME:
            el.etype = ELEMENT_LITERAL_NAME;
            el.u.name = token->u.name;
            break;
        default:
            assert_fail("CAN'T CONVERT");
            break;
    }

    return el;
}

