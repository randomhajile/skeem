typedef enum {
  OP_BODY,
  OP_BIND,
  // OP_LOAD,
  // OP_TOPLVL,
  OP_READ,
  OP_PRINT,
  OP_EVAL,
  OP_APPLY,
  OP_SAVE,
  OP_LAMBDA,
  OP_QUOTE,
  OP_DEFINE,
  OP_BEGIN,
  OP_IF,
  OP_SET,
  OP_LET,
  OP_COND,
  OP_DELAY,
  OP_AND,
  OP_OR,
  // OP_CONSSTREAM,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_REM,
  OP_CAR,
  OP_CDR,
  OP_CONS,
  OP_SETCAR,
  OP_SETCDR,
  OP_NOT,
  OP_BOOLEAN,
  OP_NULL,
  OP_ZERO,
  OP_POSP,
  OP_NEGP,
  OP_NEQ,
  OP_LT,
  OP_GT,
  OP_LTE,
  OP_GTE,
  OP_SYMBOL,
  OP_NUMBER,
  OP_STRING,
  OP_PROCEDURE,
  OP_PAIR,
  OP_EQ,
  OP_EQP,
  OP_EQV,
  OP_FORCE,
  OP_WRITE,
  OP_DISPLAY,
  OP_NEWLINE,
  OP_ERROR,
  OP_REVERSE,
  OP_APPEND,
  OP_PUT,
  OP_GET,
  OP_QUIT,
  OP_GC,
  OP_RDSEXPR,
  OP_RDLIST,
  OP_RDDOT,
  OP_RDQUOTE,
  OP_LIST,
  OP_LIST_LENGTH,
  OP_ASSQ,
  OP_GET_CLOSURE,
  OP_CLOSURE,
  OP_USER_DEFINED,
} OP_CODE;
