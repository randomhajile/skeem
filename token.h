#include "common.h"

/***************/
/* Token Codes */
/***************/

typedef enum {
  TOK_NO_TOKEN, TOK_SYMBOL, TOK_INT, TOK_FLOAT, TOK_STRING, TOK_BOOL,
  TOK_CHAR,
  TOK_QUOTE, TOK_BQUOTE, TOK_LPAREN, TOK_RPAREN, TOK_LBRACKET, TOK_RBRACKET,
  TOK_NIL, TOK_COMMA, TOK_COMMAAT,
  TOK_ERROR,
  TOK_END_OF_FILE,
} TOKEN_CODE;

typedef struct {
  TOKEN_CODE code;
  union {
    int integer;
    float real;
    char string[1024];
    char symbol[1024];
    BOOLEAN bool;
    char character;
  } value;
} TOKEN;
