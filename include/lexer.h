#ifndef LEXER_H__
#define LEXER_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

typedef enum {
    TOKEN_NUM,

    TOKEN_SIN,
    TOKEN_ABS,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULT,
    TOKEN_DIV,
    TOKEN_POWER,

    TOKEN_SHR,
    TOKEN_SHL,
    TOKEN_BITAND,
    TOKEN_BITOR,
    TOKEN_BITNOT,

    TOKEN_LEFT_PAR,
    TOKEN_RIGHT_PAR,

    TOKEN_PRINT,

    TOKEN_ERROR,
    TOKEN_EOF,
} TokenType;

typedef struct {
    char* data;
    size_t length;

    TokenType type;
} Token;

typedef struct {
    Token token;

    char* input_stream;
    size_t pos;
} Lexer;


void lexer_init(Lexer* lexer, char* input_stream);

void lexer_file_init(Lexer* lexer, const char* file_name);

void lexer_advance(Lexer* lexer);


Token make_token(char* data, size_t length, TokenType type);

Token get_token(Lexer* lexer);

Token check_keyword(Lexer* lexer, int start, int length, const char* rest, TokenType type);


bool is_digit(char symbol);

bool is_space(char symbol);

bool is_alpha(char symbol);

int to_digit(char symbol);


#endif // LEXER_H__
       

#ifdef LEXER_INCLUDE_IMPLEMENTATION

void lexer_init(Lexer* lexer, char* input_stream)
{
    lexer->input_stream = input_stream;
    lexer->pos = 0;

    lexer->token = get_token(lexer);
}

void lexer_file_init(Lexer* lexer, const char* file_name)
{
    char* buffer;
    long  length;

    FILE* file = fopen(file_name, "rb");
    assert(file);

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = malloc(length);
    assert(buffer);    

    fread(buffer, 1, length, file);
    fclose(file);

    lexer_init(lexer, buffer);
}

void lexer_advance(Lexer* lexer)
{
    lexer->token = get_token(lexer);
}


Token make_token(char* data, size_t length, TokenType type)
{   
    return (Token) { .data = data, .length = length, .type = type};
}

Token get_token(Lexer* lexer)
{
    if(lexer->input_stream[lexer->pos + 1] == '\0' || lexer->input_stream[lexer->pos + 1] == EOF)
    {
        return make_token(NULL, 0, TOKEN_EOF);
    }

    while(is_space(lexer->input_stream[lexer->pos]))
    {
        lexer->pos += 1;
    }

    switch(lexer->input_stream[lexer->pos])
    {
        case '+':
            return make_token(lexer->input_stream + lexer->pos++, 1, TOKEN_PLUS);

        case '-':
            return make_token(lexer->input_stream + lexer->pos++, 1, TOKEN_MINUS);

        case '*':
            return make_token(lexer->input_stream + lexer->pos++, 1, TOKEN_MULT);

        case '/':
            return make_token(lexer->input_stream + lexer->pos++, 1, TOKEN_DIV);

        case '(':
            return make_token(lexer->input_stream + lexer->pos++, 1, TOKEN_LEFT_PAR);

        case ')':
            return make_token(lexer->input_stream + lexer->pos++, 1, TOKEN_RIGHT_PAR);

        case '^':
            return make_token(lexer->input_stream + lexer->pos++, 1, TOKEN_POWER);

        case '&':
            return make_token(lexer->input_stream + lexer->pos++, 1, TOKEN_BITAND);

        case '|':
            return make_token(lexer->input_stream + lexer->pos++, 1, TOKEN_BITOR);

        case '~':
            return make_token(lexer->input_stream + lexer->pos++, 1, TOKEN_BITNOT);

        case '<':
            if(lexer->input_stream[lexer->pos + 1] == '<')
            {
                Token tok = make_token(lexer->input_stream + lexer->pos, 2, TOKEN_SHL);
                lexer->pos += 2;

                return tok;
            } 
            break;

        case '>':
            if(lexer->input_stream[lexer->pos + 1] == '>')
            {
                Token tok = make_token(lexer->input_stream + lexer->pos, 2, TOKEN_SHR);
                lexer->pos += 2;

                return tok;
            }
            break;

        default: 
            break;
    }

    if(is_alpha(lexer->input_stream[lexer->pos]))
    {
        switch(lexer->input_stream[lexer->pos])
        {
            case 'a':
                return check_keyword(lexer, 1, 2, "bs", TOKEN_ABS);

            case 's':
                return check_keyword(lexer, 1, 2, "in", TOKEN_SIN);

            case 'p': 
                return check_keyword(lexer, 1, 4, "rint", TOKEN_PRINT);

            default:
                break;
        }
    }

    if(is_digit(lexer->input_stream[lexer->pos]))
    {
        Token new_number = make_token(lexer->input_stream + lexer->pos, 1, TOKEN_NUM);

        while(is_digit(lexer->input_stream[++lexer->pos]))
        {
            new_number.length += 1;
        }

        return new_number;
    }

    return make_token(NULL, 0, TOKEN_ERROR);
}

Token check_keyword(Lexer* lexer, int start, int length, const char* rest, TokenType type)
{
    if(memcmp(lexer->input_stream + lexer->pos + start, rest, length) == 0)
    {
        Token new_keyword = make_token(lexer->input_stream + lexer->pos, length + 1, type);
        lexer->pos += length + 1;

        return new_keyword;
    } else {
        return make_token(NULL, 0, TOKEN_ERROR);
    }
}

bool is_digit(char symbol)
{
    return symbol >= '0' && symbol <= '9';
}

bool is_space(char symbol)
{
    return symbol == ' ' || symbol == '\t' || symbol == '\n';
}

bool is_alpha(char symbol)
{
    return (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z');
}

int to_digit(char symbol)
{
    return symbol - '0';
}


#endif // LEXER_INCLUDE_IMPLEMENTATION

