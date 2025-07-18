#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include <sys/mman.h>

#define LEXER_INCLUDE_IMPLEMENTATION
#include "lexer.h"

static char input_buffer[256];

typedef enum {
    OP_ADD,
    OP_SUB,
} OpType;

typedef struct {
    uint32_t operand;
    OpType   op_kind;
} Op; 

typedef struct {
    size_t length;
    Op*    commands;
} Bytecode;

typedef uint32_t (*jitfunc_t)(void);

void append_op(Bytecode* code, Op command)
{
    if(code->commands == NULL)
    {
        code->length = 0;
    }

    code->length += 1;

    code->commands = (Op*)realloc(code->commands, code->length * sizeof(Op));
    code->commands[code->length - 1] = command;
}

void gen_bytecode(char* input, Bytecode* bytecode)
{
    Lexer lexer;
    lexer_init(&lexer, input);

    while(lexer.token.type != TOKEN_EOF)
    {
        Op new_op = {0};

        switch(lexer.token.type)
        {
            case TOKEN_PLUS:
                new_op.op_kind = OP_ADD;
                break;

            case TOKEN_MINUS:
                new_op.op_kind = OP_SUB;
                break;

            default:
                assert(0 && "Unreachable");
        }

        lexer_advance(&lexer);
        assert(lexer.token.type == TOKEN_NUM);

        for(size_t i = 0; i < lexer.token.length; ++i)
        {
            new_op.operand = new_op.operand * 10 + to_digit(lexer.token.data[i]);
        }

        append_op(bytecode, new_op);

        lexer_advance(&lexer);
    }
}

jitfunc_t get_jitfunc(Bytecode* code)
{
        /* +3 is an offset for xor rax, rax
         * +1 is an offset for ret
         *
         * TODO: Use adequate dynamic array instead of this bullshit! 
         */
        uint8_t* jit_code = (uint8_t*)malloc(code->length * 6 + 3 + 1); 

        // xor rax, rax
        jit_code[0] = 0x48;
        jit_code[1] = 0x31;
        jit_code[2] = 0xc0;

        for(size_t i = 0; i < code->length; ++i)
        {
            switch(code->commands[i].op_kind)
            {
                case OP_ADD:
                {
                    // add rax, 
                    jit_code[i * 6 + 3 + 0] = 0x48;
                    jit_code[i * 6 + 3 + 1] = 0x05;

                    break;
                }

                case OP_SUB:
                {
                    // sub rax, 
                    jit_code[i * 6 + 3 + 0] = 0x48;
                    jit_code[i * 6 + 3 + 1] = 0x2d;

                    break;
                }

                default:
                    assert(0 && "Unreachable");
            }

            // uint32_t to little-endian
            jit_code[i * 6 + 3 + 2] = code->commands[i].operand         & 0xff;
            jit_code[i * 6 + 3 + 3] = (code->commands[i].operand >> 8)  & 0xff;
            jit_code[i * 6 + 3 + 4] = (code->commands[i].operand >> 16) & 0xff;
            jit_code[i * 6 + 3 + 5] = (code->commands[i].operand >> 24) & 0xff;
        }

        // ret
        jit_code[code->length * 6 + 3] = 0xc3; 

        void* exec_code = mmap(NULL, code->length * 6 + 3 + 1, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if(exec_code == MAP_FAILED)
        {
            printf("Couldn't map memory page!\n");
            exit(EXIT_FAILURE);
        }

        memcpy(exec_code, jit_code, code->length * 6 + 3 + 1);
        free(jit_code);

        return (jitfunc_t)exec_code;
}

uint32_t run_jit(Bytecode* code)
{
    jitfunc_t func = get_jitfunc(code);
    
    uint32_t ret_value = func();

    munmap(func, code->length * 6 + 3 + 1);

    free(code->commands);
    code->commands = NULL;

    return ret_value;
}

int main()
{
    while(true)
    {
        fputs("> ", stdout);

        fgets(input_buffer, 256, stdin);

        Bytecode code = {0};
        gen_bytecode(input_buffer, &code);
            
        printf("Computed value: %d\n", run_jit(&code));
    }

    return 0;
}
