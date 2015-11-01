#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "tims.h"

// TODO: long strings/symbols

#define PROMPT "~ "

/***********/
/* Globals */
/***********/

LISP_OBJ_PTR    object_pool[MAX_NUM_OBJS];
ENVIRONMENT_PTR environment_pool[MAX_NUM_ENVS];
STACK_FRAME     stack[MAX_NUM_FRAMES];
// next_frame is the frame that will be pushed
STACK_FRAME_PTR next_frame = stack;
// current_frame is the frame to be evaluated.
STACK_FRAME_PTR current_frame = NULL;

// testing
FILE *in_stream;
FILE *out_stream;

LISP_OBJ nil;
LISP_OBJ_PTR nil_ptr = &nil;
LISP_OBJ _false;
LISP_OBJ _true;
LISP_OBJ_PTR false_ptr = &_false;
LISP_OBJ_PTR true_ptr = &_true;
ENVIRONMENT global_env;
ENVIRONMENT_PTR global_env_ptr = &global_env;
ENVIRONMENT_PTR debug;

/**************/
/* For lexing */
/**************/

#define char_code(ch) char_table[ch]
#define MAX_LINE_SIZE 1024

typedef enum {
  LETTER, DIGIT,
  LPAREN, RPAREN,
  QUOTE, BQUOTE, DQUOTE, SEMICOLON, HASH,
  COMMA,
  WHITESPACE,
  EOFCHAR,
} CHAR_CODE;

CHAR_CODE char_table[256];

TOKEN current_token;   // the current token being read

char current_line[MAX_LINE_SIZE];
char *end_of_line  = current_line;
char *next_char    = current_line;
char current_char;


void init_char_table() {
  int ch;

  // Initialize character table
  // allow for some crazy symbols, like \_1
  for (ch = 0; ch < 256; ++ch)
    char_table[ch] = LETTER;
  for (ch = '0'; ch <= '9'; ++ch)
    char_table[ch] = DIGIT;

  char_table['\'']     = QUOTE;
  char_table['`']      = BQUOTE;
  char_table['"']      = DQUOTE;
  char_table['(']      = LPAREN;
  char_table[')']      = RPAREN;
  char_table[',']      = COMMA;
  char_table[' ']      = WHITESPACE;
  char_table['\t']     = WHITESPACE;
  char_table['\n']     = WHITESPACE;
  char_table[';']      = SEMICOLON;
  char_table['#']      = HASH;
  char_table[EOF]      = EOFCHAR;
}

void token() {

  skip_whitespace();
  switch(char_code(get_char())) {
  case LPAREN:
    current_token.code = TOK_LPAREN;
    break;
  case RPAREN:
    current_token.code = TOK_RPAREN;
    break;
  case QUOTE:
    current_token.code = TOK_QUOTE;
    break;
  case DQUOTE:
    current_token.code = TOK_STRING;
    get_string();
    break;
  case BQUOTE:
    current_token.code = TOK_BQUOTE;
    break;
  case COMMA:
    if (get_char() == '@')
      current_token.code = TOK_COMMAAT;
    else {
      put_back();
      current_token.code = TOK_COMMA;
    }
    break;
  case HASH:
    get_char();
    if (current_char == '\\') {
      current_token.code = TOK_CHAR;
      get_s_char();
    } else if (current_char == 'f') {
      current_token.code = TOK_BOOL;
      current_token.value.bool = FALSE;
    } else if (current_char == 't') {
      current_token.code = TOK_BOOL;
      current_token.value.bool = TRUE;
    } else {
      // TODO: there has been an error and we need to do something...
      fprintf(stderr, "Invalid # express.");
    }
    break;
  case SEMICOLON:
    skip_comment();
    token();
    break;
  default:
    current_token.code = TOK_NO_TOKEN;
    // put_back();
    get_atom();
  }
}

void skip_whitespace() {
  while (char_code(get_char()) == WHITESPACE) {

  }
  put_back();
}

char get_char() {
  // check to see if we need to read into the input buffer
  if (next_char >= end_of_line) {
    // the input buffer is empty and we need to read more or stop
    // if (fgets(next_char = current_line, MAX_LINE_SIZE, stdin) == NULL) {
    if (fgets(next_char = current_line, MAX_LINE_SIZE, in_stream) == NULL) {
      if (in_stream == stdin) {
        fprintf(stderr, "Bye for now!\n");
        exit(0);
      } else {
        fclose(in_stream);
        fclose(out_stream);
        in_stream = stdin;
        out_stream = stdout;
        fprintf(out_stream, PROMPT);
      }
    }
    end_of_line = current_line + strlen(current_line);
  }
  current_char = *next_char++;
  return current_char;
}

void put_back() {
  // puts the current character back.
  // does not check if we're going to fall off the edge of the world, so should
  // only be called after we've done get_char(). Slight issue on certain types
  // of tokens, but those will become syntax errors later.
  next_char--;
  current_char = *(next_char - 1);
}

void skip_comment() {
  while (get_char() != '\n'){

  }
}

void get_atom() {
  // first, try to get a number
  get_number();
  if (current_token.code == TOK_NO_TOKEN)
    get_sym();
}

void get_s_char() {
  // for now, don't allow names.
  current_token.value.character = get_char();
}

void get_string() {
  char *sp = current_token.value.string;
  int string_length = 0;

  while (get_char() != '"') {
    if (current_char == '\\') {
      string_length++;
      // only allowing escaped DQUOTE for now
      if (get_char() != '"') {
        // TODO: And error goes here
      }
    }
    string_length++;
    *sp++ = current_char;
  }
  *sp = '\0';
}

void get_sym() {
  // extracts a symbol
  int sym_length = 0;

  while ((char_code(current_char) == LETTER) || (char_code(current_char) == DIGIT)) {
    current_token.value.symbol[sym_length++] = current_char;
    get_char();
  }
  put_back();
  current_token.code = TOK_SYMBOL;
  current_token.value.symbol[sym_length] = '\0';
}

void get_number() {
  char num_string[1024];
  int num_chars = 0;
  BOOLEAN saw_decimal = FALSE;
  BOOLEAN is_exponential = FALSE;

  if (current_char == '-') {
    num_string[num_chars++] = '-';
    get_char();
    if (char_code(current_char) != DIGIT && current_char != '.' && current_char != 'e' && current_char != 'E') {
      put_back();
      return;
    }
  }

  while (char_code(current_char) == DIGIT || current_char == '.' || current_char == 'e' || current_char == 'E') {
    num_string[num_chars] = current_char;
    num_chars++;
    // need some error checking here
    if (current_char == '.' && !saw_decimal && !is_exponential)
      saw_decimal = TRUE;
    else if (current_char == 'e' || current_char == 'E')
      is_exponential = TRUE;
    get_char();
  }
  if (current_char != ')' && char_code(current_char) != WHITESPACE) {
    while (num_chars--)
      put_back();
    return;
  }
  put_back();
  num_string[num_chars] = '\0';
  if (saw_decimal || is_exponential) {
    current_token.code = TOK_FLOAT;
    current_token.value.real = atof(num_string);
  } else {
    current_token.code = TOK_INT;
    current_token.value.integer = atoi(num_string);
  }
}

/************************/
/* For the Lisp reader. */
/************************/
LISP_OBJ_PTR read(LISP_OBJ_PTR read_to) {
  token();
  return read_sexp(read_to);
}


LISP_OBJ_PTR read_sexp(LISP_OBJ_PTR read_to) {
  LISP_OBJ_PTR res = read_to;

  switch (current_token.code) {
  case TOK_LPAREN:
    form(res) = CONS_FORM;
    return read_list(res);
  case TOK_INT:
    form(res) = INT_FORM;
    int_value(res) = current_token.value.integer;
    break;
  case TOK_FLOAT:
    form(res) = FLOAT_FORM;
    float_value(res) = current_token.value.real;
    break;
  case TOK_STRING:
    form(res) = STRING_FORM;
    string_value(res) = malloc(strlen(current_token.value.string)+1);
    strcpy(string_value(res), current_token.value.string);
    break;
  case TOK_BOOL:
    form(res) = BOOLEAN_FORM;
    bool_value(res) = current_token.value.bool;
    break;
  case TOK_CHAR:
    form(res) = CHAR_FORM;
    char_value(res) = current_token.value.character;
    break;
  case TOK_SYMBOL:
    form(res) = SYMBOL_FORM;
    symbol_value(res) = malloc(strlen(current_token.value.string)+1);
    strcpy(symbol_value(res), current_token.value.symbol);
    break;
  case TOK_QUOTE:
    // We're treating 'foo as syntactic sugar that maps to (quote foo).
    // this means we have to take something like '(1 2 3) and transform it into
    // (quote (1 2 3)) and 'a into (quote a).
    form(res) = CONS_FORM;
    car(res) = NULL;
    cdr(res) = NULL;
    car(res) = alloc_obj();
    form(car(res)) = SYMBOL_FORM;
    symbol_value(car(res)) = malloc((strlen("quote") + 1)*sizeof(char));
    strcpy(symbol_value(car(res)), "quote");
    cdr(res) = alloc_obj();
    form(cdr(res)) = CONS_FORM;
    cadr(res) = alloc_obj();
    cddr(res) = nil_ptr;
    cadr(res) = read(cadr(res));
    break;
  default:
    // TODO: implement something for the following
    // TODO: TOK_NO_TOKEN
    // TODO: TOK_RPAREN
    // TODO: TOK_LBRACKET
    // TODO: TOK_RBRACKET
    // TODO: TOK_END_OF_FILE
    break;
  }

  return res;
}

LISP_OBJ_PTR read_list(LISP_OBJ_PTR root) {
  LISP_OBJ_PTR current_obj = root;
  LISP_OBJ_PTR previous_obj;

  token();
  // special case for '()
  if (current_token.code == TOK_RPAREN)
    return nil_ptr;

  while (TRUE) {
    current_obj->value.cons.car = NULL;
    current_obj->value.cons.car = alloc_obj();
    // again, this is silly, but takes care of the nil case.
    car(current_obj) = read_sexp(car(current_obj));
    // current_obj->value.cons.car = read_sexp(current_obj->value.cons.car);
    token();
    // have to break so we don't over-allocate.
    if (current_token.code == TOK_RPAREN)
      break;
    previous_obj = current_obj;
    current_obj = alloc_obj();
    current_obj->form = CONS_FORM;
    previous_obj->value.cons.cdr = current_obj;
  }
  current_obj->value.cons.cdr = nil_ptr;

  return root;
}


BOOLEAN init_memory() {
  int i;

  for (i = 0; i < MAX_NUM_OBJS; ++i)
    object_pool[i] = malloc(sizeof(LISP_OBJ));
  for (i = 0; i < MAX_NUM_ENVS; ++i) {
    environment_pool[i] = malloc(sizeof(ENVIRONMENT));
    environment_pool[i]->in_use = FALSE;
    // let's explicitly initialize these to NULL, since we will use their
    // NULLness to indicate that we're at the end of a search.
    environment_pool[i]->root = NULL;
    environment_pool[i]->enclosing_env = NULL;
  }
  false_ptr->form = BOOLEAN_FORM;
  false_ptr->value.atom.bool_value = FALSE;
  true_ptr->form = BOOLEAN_FORM;
  true_ptr->value.atom.bool_value = TRUE;

  return TRUE;
}

BOOLEAN init_global_env() {
  LISP_OBJ_PTR func;

  make_primitive("read", OP_READ);
  make_primitive("display", OP_DISPLAY);
  make_primitive("eval", OP_EVAL);
  make_primitive("apply", OP_APPLY);
  make_primitive("begin", OP_BEGIN);
  make_primitive("let", OP_LET);
  make_primitive("+", OP_ADD);
  make_primitive("-", OP_SUB);
  make_primitive("*", OP_MUL);
  make_primitive("/", OP_DIV);
  make_primitive("car", OP_CAR);
  make_primitive("cdr", OP_CDR);
  make_primitive("cons", OP_CONS);
  make_primitive("set-car!", OP_SETCAR);
  make_primitive("set-cdr!", OP_SETCDR);
  make_primitive("define", OP_DEFINE);
  make_primitive("quote", OP_QUOTE);
  make_primitive("if", OP_IF);
  make_primitive("or", OP_OR);
  make_primitive("and", OP_AND);
  make_primitive("=", OP_EQ);
  make_primitive("eq?", OP_EQP);
  make_primitive("<", OP_LT);
  make_primitive("<=", OP_LTE);
  make_primitive(">", OP_GT);
  make_primitive(">=", OP_GTE);
  make_primitive("lambda", OP_LAMBDA);
  make_primitive("set!", OP_SET);
  make_primitive("pair?", OP_PAIR);
  make_primitive("zero?", OP_ZERO);
  make_primitive("procedure?", OP_PROCEDURE);
  make_primitive("closure?", OP_CLOSURE);
  make_primitive("string?", OP_STRING);
  make_primitive("number?", OP_NUMBER);
  make_primitive("symbol?", OP_SYMBOL);
  make_primitive("null?", OP_NULL);
  make_primitive("boolean?", OP_BOOLEAN);

  in_stream = fopen("std_lib.l", "r");
  out_stream = fopen("/dev/null", "w");

  return TRUE;
}

void print_lispobj(LISP_OBJ_PTR objp) {
  if (objp == nil_ptr) {
    fprintf(out_stream, "nil");
    return;
  }

  switch (objp->form) {
  case INT_FORM:
    fprintf(out_stream, "%d", int_value(objp));
    break;
  case FLOAT_FORM:
    fprintf(out_stream, "%g", float_value(objp));
    break;
  case CHAR_FORM:
    fprintf(out_stream, "#\\%c", char_value(objp));
    break;
  case STRING_FORM:
    fprintf(out_stream, "\"%s\"", string_value(objp));
    break;
  case SYMBOL_FORM:
    fprintf(out_stream, "%s", symbol_value(objp));
    break;
  case PROCEDURE_FORM:
    fprintf(out_stream, "<PROCEDURE>");
    break;
  case BOOLEAN_FORM:
    fprintf(out_stream, "#%c", bool_value(objp) ? 't' : 'f');
    break;
  case CONS_FORM:
    fprintf(out_stream, "(");
    while (TRUE) {
      print_lispobj(car(objp));
      objp = cdr(objp);
      if (objp == nil_ptr)
        break;
      if (!(is_pair(objp))) {
        fprintf(out_stream, " . ");
        print_lispobj(objp);
        break;
      }
      fprintf(out_stream, " ");
    }
    fprintf(out_stream, ")");
    break;
  case NO_FORM:
    fprintf(out_stream, "no form, boss");
    break;
  default:
    fprintf(out_stream, "dunno that form %d", form(objp));
  }
}


void eval_cycle() {
  while (TRUE) {
    next_code = OP_PRINT;
    next_env = global_env_ptr;
    next_args = nil_ptr;
    next_ret = NULL;
    push();
    // eval
    next_ret = &(current_args);
    next_code = OP_EVAL;
    next_env = current_env;
    push();
    // read
    next_ret = &(current_args);
    next_code = OP_READ;
    next_env = current_env;
    next_args = nil_ptr;
    push();
    fprintf(out_stream, PROMPT);
    eval_stack();
  }
}


void eval_stack() {
  // This will run through the stack until we're finished with the current eval.
  while (current_frame > stack) {
    while (current_res != NULL)
      pop();
    eval_current_frame();
  }
  pop();
  fprintf(out_stream, "\n");
}

void eval_current_frame() {
  LISP_OBJ_PTR args = current_args;
  int num_args = 0;

  switch (current_code) {
  case OP_READ:
    current_res = alloc_obj();
    current_res = read(current_res);
    break;
  case OP_EVAL:
    current_res = eval_lispobj(current_args);
    break;
  case OP_PRINT:
    print_lispobj(current_args);
    current_res = nil_ptr;
    break;
  case OP_APPLY:
    // This should never be true when called from the interpreter, only possible
    // if apply is called by the user.
    if (!is_pair(current_args) && !is_null(current_args)) {
      return error_msg("ERROR -- second arg to apply must be a list");
    }
    current_res = apply_func(current_args);
    break;
  case OP_BIND:
    // not user accessible
    current_res = nil_ptr;
    bind_funargs(current_env, current_func, current_args);
    break;
  case OP_LET:
    current_res = apply_let();
    break;
  case OP_DISPLAY:
    current_res = true_ptr;
    display(car(current_args));
    return;
  case OP_BEGIN:
    current_res = apply_begin();
    break;
  case OP_ERROR:
    error();
    break;
  case OP_SAVE:
    current_res = nil_ptr;
    break;
  case OP_QUOTE:
    current_res = current_args;
    break;
  case OP_IF:
    current_res = apply_if();
    break;
  case OP_OR:
    current_res = apply_or();
    break;
  case OP_AND:
    current_res = apply_and();
    break;
  case OP_EQP:
    if (list_length(current_args) != 2)
      return error_msg("ERROR -- eq? takes precisely two arguments");
    if (car(current_args) == cadr(current_args))
      current_res = true_ptr;
    else
      current_res = false_ptr;
    break;
  case OP_EQ:
    // all of the number ops should be implemented as accumulators
    if (!check_all_number(current_args))
      return error_msg("ERROR -- arguments for = must be number type");
    current_res = eq(current_args);
    break;
  case OP_LT:
    if (!check_all_number(current_args))
      return error_msg("ERROR -- arguments for < must be number type");
    current_res = less_than(current_args);
    break;
  case OP_LTE:
    if (!check_all_number(current_args))
      return error_msg("ERROR -- arguments for <= must be number type");
    current_res = less_than_eq(current_args);
    break;
  case OP_GT:
    if (!check_all_number(current_args))
      return error_msg("ERROR -- arguments for > must be number type");
    current_res = greater_than(current_args);
    break;
  case OP_GTE:
    if (!check_all_number(current_args))
      return error_msg("ERROR -- arguments for >= must be number type");
    current_res = greater_than_eq(current_args);
    break;
  case OP_ADD:
    if (!check_all_number(current_args))
      return error_msg("ERROR -- arguments for + must be number type");
    current_res = add(current_args);
    break;
  case OP_SUB:
    if (!check_all_number(current_args))
      return error_msg("ERROR -- arguments for - must be number type");
    current_res = sub(current_args);
    break;
  case OP_MUL:
    if (!check_all_number(current_args))
      return error_msg("ERROR -- arguments for * must be number type");
    current_res = mult(current_args);
    break;
  case OP_DIV:
    if (!check_all_number(current_args))
      return error_msg("ERROR -- arguments for / must be number type");
    current_res = divide(current_args);
    break;
  case OP_LAMBDA:
    current_res = make_lambda(current_env, current_args);
    break;
  case OP_SET:
    if (search_environment(current_env, car(current_args)->value.atom.symbol_value) == NULL)
      return error_msg("ERROR -- unbound variable");
    current_res = set(current_env, current_args);
    break;
  case OP_CAR:
    while (args != nil_ptr) {
      if (num_args)
        return error_msg("ERROR -- car takes precisely one argument");

      if (!is_pair(car(args)))
        return error_msg("ERROR -- argument for car must be a pair");

      args = cdr(args);
      ++num_args;
    }
    if (!num_args)
      return error_msg("ERROR -- car takes precisely one argument");

    current_res = caar(current_args);
    break;
  case OP_CDR:
    while (args != nil_ptr) {
      if (num_args) {
        error_msg("ERROR -- cdr takes precisely one argument");
        return;
      }
      if (!is_pair(car(args)))
        return error_msg("ERROR -- argument for cdr must be a pair");

      args = cdr(args);
      ++num_args;
    }

    if (!num_args)
      return error_msg("ERROR -- cdr takes precisely one argument");

    current_res = cdar(current_args);
    break;
  case OP_CONS:
    while (args != nil_ptr) {
      ++num_args;
      args = cdr(args);
    }
    if (num_args != 2)
      return error_msg("ERROR -- cons takes precisely two arguments");

    current_res = cons(car(current_args), cadr(current_args));
    break;
  case OP_SETCAR:
    // similar comments to above
    if (!is_pair(car(args)))
      return error_msg("ERROR -- first arg to set-car! must be a pair");

    while (args != nil_ptr) {
      ++num_args;
      args = cdr(args);
    }
    if (num_args != 2)
      return error_msg("ERROR -- set-car! takes precisely two arguments");

    current_res = set_car(car(current_args), car(cdr(current_args)));
    break;
  case OP_SETCDR:
    if (!is_pair(car(args)))
      return error_msg("ERROR -- first arg to set-cdr! must be a pair");

    while (args != nil_ptr) {
      ++num_args;
      args = cdr(args);
    }
    if (num_args != 2)
      return error_msg("ERROR -- set-cdr! takes precisely two arguments");

    current_res = set_cdr(car(current_args), car(cdr(current_args)));
    break;
  case OP_DEFINE:
    current_res = define(current_env, current_args);
    break;
  case OP_PAIR:
    if (is_pair(car(current_args)))
      current_res = true_ptr;
    else
      current_res = false_ptr;
    break;
  case OP_ZERO:
    if (is_int(car(current_args))) {
      if (int_value(car(current_args)) == 0)
        current_res = true_ptr;
      else
        current_res = false_ptr;
    } else if (form(car(current_args)) == FLOAT_FORM) {
      if (float_value(car(current_args)) == 0.0)
        current_res = true_ptr;
      else
        current_res = false_ptr;
    } else
      current_res = false_ptr;
    break;
  case OP_PROCEDURE:
  case OP_CLOSURE:
    if (check_type(car(current_args), PROCEDURE_FORM))
      current_res = true_ptr;
    else
      current_res = false_ptr;
    break;
  case OP_SYMBOL:
    if (check_type(car(current_args), SYMBOL_FORM))
      current_res = true_ptr;
    else
      current_res = false_ptr;
    break;
  case OP_NUMBER:
    if (is_number(car(current_args)))
      current_res = true_ptr;
    else
      current_res = false_ptr;
    break;
  case OP_STRING:
    if (is_string(car(current_args)))
      current_res = true_ptr;
    else
      current_res = false_ptr;
    break;
  case OP_NULL:
    if (is_null(car(current_args)))
      current_res = true_ptr;
    else
      current_res = false_ptr;
    break;
  case OP_BOOLEAN:
    if (is_bool(car(current_args)))
      current_res = true_ptr;
    else
      current_res = false_ptr;
    break;
    // default:
    // printf("unknown op %d, boss\n", current_code);
  }
}

LISP_OBJ_PTR eval_lispobj(LISP_OBJ_PTR objp) {
  LISP_OBJ_PTR res = NULL;
  LISP_OBJ_PTR func, args, args_list;
  ENVIRONMENT_PTR eval_env = current_env;
  STACK_FRAME_PTR frame = current_frame;

  if (objp == nil_ptr)
    return objp;

  switch (objp->form) {
  case INT_FORM:
  case FLOAT_FORM:
  case CHAR_FORM:
  case STRING_FORM:
  case BOOLEAN_FORM:
  case PROCEDURE_FORM:
    return objp;
  case SYMBOL_FORM:
    while (res == NULL && frame >= stack) {
      res = search_environment(frame->env, symbol_value(objp));
      if (frame == stack)
        break;
      --frame;
    }
    if (res == NULL) {
      fprintf(stderr, "Unknown variable %s", symbol_value(objp));
      error();
      return NULL;
    }
    return res;
  case CONS_FORM:
    // set the current op to apply
    current_code = OP_APPLY;
    // evaluate the function so that we can apply it.
    next_ret = &(current_func);
    next_code = OP_EVAL;
    next_env = eval_env;
    next_args = car(current_args);
    current_args = cdr(current_args);
    push();

    return NULL;
  }
}

void eval_args(LISP_OBJ_PTR args) {
  if (is_null(current_args))
    return;

  switch (current_code) {
  case OP_QUOTE:
    current_args = car(current_args);
    // do not eval any of the args
    return;
  case OP_BEGIN:
  case OP_LET:
    return;
  case OP_DEFINE:
    if (is_symbol(car(args)))
      args = cdr(args);
    else
      args = nil_ptr;
    break;
  case OP_LAMBDA:
    args = nil_ptr;
    break;
  case OP_IF:
    next_ret = &(car(args));
    next_code = OP_EVAL;
    next_env = current_env;
    next_args = car(args);
    push();
    args = nil_ptr;
    break;
  case OP_SET:
    args = cdr(args);
    break;
  case OP_EVAL:
    args = car(args);
    next_ret = &(current_args);
    next_code = OP_EVAL;
    next_env = current_env;
    next_args = args;
    push();
    return;
  case OP_OR:
    next_ret = &(car(current_args));
    next_code = OP_EVAL;
    next_args = car(current_args);
    next_env = current_env;
    push();
    return;
  case OP_APPLY:
    args = current_args;
    // this is a hack, but we need to get the func before proceeding
    if (is_symbol(car(args)) || is_pair(car(args))) {
      next_code = OP_EVAL;
      next_ret = &(car(current_args));
      next_args = car(args);
      push();
      return;
    } else if (!is_proc(car(args))) {
      error_msg("ERROR -- first arg to apply must be a function");
      return;
    }
    if (is_special(car(args))) {
      current_code = proc_code(car(args));
      next_code = OP_EVAL;
      next_ret = &(current_args);
      next_args = cadr(args);
      next_env = current_env;
      push();
    } else {
      /* current_func = car(args); */
      /* next_code = OP_EVAL; */
      /* next_ret = &current_args; */
      /* next_args = cadr(current_args); */
      /* next_env = current_env; */
      /* push() */

      current_code = OP_BEGIN;
      current_func = car(args);
      current_args = proc_body(current_func);
      current_env = alloc_environment();
      current_env->in_use = TRUE;
      current_env->enclosing_env = proc_env(current_func);
      debug = current_env;
      next_code = OP_BIND;
      next_ret = NULL;
      next_res = NULL;
      next_args = nil_ptr;
      next_func = current_func;
      next_env = current_env;
      push();
      next_code = OP_EVAL;
      next_ret = &current_args;
      next_args = cadr(args);
      next_env = current_env;
      push();
    }
    return;
  }

  while (args != nil_ptr) {
    next_ret = &(args->value.cons.car);
    next_code = OP_EVAL;
    next_env = current_env;
    next_args = car(args);
    push();
    args = cdr(args);
  }
}

void error_msg(char *msg) {
  // TODO: replace old error() calls with this.
  fprintf(stderr, "%s", msg);
  while (current_frame > stack)
    pop();
}

void error() {
  // pop everything from the stack immediately without evaluation.
  // this should cause us to fall out of the eval_stack call and back to
  // eval_cycle
  while (current_frame > stack)
    pop();
}

LISP_OBJ_PTR apply_func(LISP_OBJ_PTR objp) {
  LISP_OBJ_PTR func = current_func;
  // hack hack hack
  // TODO: replace this with an OP_SAVE

  LISP_OBJ_PTR hack = current_args;
  current_args = alloc_obj();
  form(current_args) = CONS_FORM;
  car(current_args) = hack;
  cdr(current_args) = NULL;
  cdr(current_args) = alloc_obj();
  LISP_OBJ_PTR args = copy_list(objp, current_args);
  current_args = car(current_args);

  // end hack hack hack

  if (!is_proc(func)) {
    error_msg("ERROR -- not a function.");
    return NULL;
  }

  if (is_special(func)) {
    current_code = proc_code(func);
    current_args = args;
    eval_args(args);

    return NULL;
  }

  // we need to save the current environment from getting GC'd
  next_code = OP_SAVE;
  next_env = current_env;
  next_args = nil_ptr;
  next_ret = NULL;
  push();
  frame_code((current_frame-1)) = OP_BEGIN;
  frame_env((current_frame-1)) = alloc_environment();
  frame_env((current_frame-1))->in_use = TRUE;
  frame_env((current_frame-1))->enclosing_env = proc_env(func);
  frame_args((current_frame-1)) = proc_body(func);

  eval_funargs(frame_env((current_frame-1)), current_env, func, args);

  return NULL;
}

void eval_funargs(ENVIRONMENT_PTR env,      // where to store results
                  ENVIRONMENT_PTR eval_env, // where to eval results
                  LISP_OBJ_PTR func,
                  LISP_OBJ_PTR args) {
  // I coded myself into a corner and this is the best solution I have right
  // now. I would like it to be better.
  // We need to interleave stack frames
  LISP_OBJ_PTR define_args;
  LISP_OBJ_PTR req_params = proc_reqparams(func);
  LISP_OBJ_PTR opt_params = proc_optparams(func);
  LISP_OBJ_PTR rest_params = proc_restparams(func);
  next_code = OP_SAVE;
  next_args = args;
  next_env = eval_env;
  next_ret = NULL;
  push();

  while (req_params != nil_ptr && args != nil_ptr) {
    next_code = OP_DEFINE;
    next_env = env;
    // next_args = define_args;
    next_ret = NULL;
    // doesn't work if arg n+1's value depends on arg n's value.
    define_args = alloc_obj();
    next_args = define_args;
    form(define_args) = CONS_FORM;
    car(define_args) = car(req_params);
    cdr(define_args) = NULL;
    mark_obj(define_args);
    cdr(define_args) = alloc_obj();
    form(cdr(define_args)) = CONS_FORM;
    cadr(define_args) = car(args);
    cddr(define_args) = nil_ptr;
    push();

    next_code = OP_EVAL;
    next_env = eval_env;
    next_args = car(args);
    next_ret = &(cadr(define_args));
    push();

    req_params = cdr(req_params);
    args = cdr(args);
  }

  // too few args
  if (req_params != nil_ptr) {
    error_msg("ERROR -- too few arguments");
    return;
  } else if (is_null(opt_params) && is_null(rest_params) && args != nil_ptr) {
    error_msg("ERROR -- too many arguments");
    return;
  }

  while (opt_params != nil_ptr) {
    next_code = OP_DEFINE;
    next_env = env;
    next_ret = NULL;
    // doesn't work if arg n+1's value depends on arg n's value.
    define_args = alloc_obj();
    next_args = define_args;
    form(define_args) = CONS_FORM;
    car(define_args) = car(opt_params);
    cdr(define_args) = NULL;
    // TODO: I don't think this is necessary
    mark_obj(define_args);
    cdr(define_args) = alloc_obj();
    form(cdr(define_args)) = CONS_FORM;
    if (!is_null(args))
      cadr(define_args) = car(args);
    else
      cadr(define_args) = nil_ptr;
    cddr(define_args) = nil_ptr;
    push();

    next_code = OP_EVAL;
    next_env = eval_env;
    if (!is_null(args)) {
      next_args = car(args);
      args = cdr(args);
    }
    else
      next_args = nil_ptr;
    next_ret = &(cadr(define_args));
    push();

    opt_params = cdr(opt_params);
  }

  // some error stuff should go here
  if (is_null(rest_params))
    return;

  next_code = OP_DEFINE;
  next_env = env;
  next_ret = NULL;
  next_args = alloc_obj();
  form(next_args) = CONS_FORM;
  car(next_args) = proc_restparams(func);
  // TODO: there's a risk of GC destroying args here.
  cdr(next_args) = alloc_obj();
  form(cdr(next_args)) = CONS_FORM;
  cddr(next_args) = nil_ptr;
  if (is_null(args)) {
    cadr(next_args) = nil_ptr;
    push();
    return;
  }
  cadr(next_args) = alloc_obj();
  form(cadr(next_args)) = CONS_FORM;
  LISP_OBJ_PTR rest_list = cadr(next_args);
  push();
  while (TRUE) {
    next_code = OP_EVAL;
    next_env = eval_env;
    next_ret = &(car(rest_list));
    next_args = car(args);
    push();
    args = cdr(args);
    if (args == nil_ptr)
      break;
    cdr(rest_list) = alloc_obj();
    rest_list = cdr(rest_list);
    form(rest_list) = CONS_FORM;
  }
  cdr(rest_list) = nil_ptr;
}

void bind_funargs(ENVIRONMENT_PTR env,      // where to store results
                  LISP_OBJ_PTR func,
                  LISP_OBJ_PTR args) {
  // I coded myself into a corner and this is the best solution I have right
  // now. I would like it to be better.
  // We need to interleave stack frames
  LISP_OBJ_PTR define_args;
  LISP_OBJ_PTR req_params = proc_reqparams(func);
  LISP_OBJ_PTR opt_params = proc_optparams(func);
  LISP_OBJ_PTR rest_params = proc_restparams(func);

  while (req_params != nil_ptr && args != nil_ptr) {
    next_code = OP_DEFINE;
    next_env = env;
    next_ret = NULL;
    // doesn't work if arg n+1's value depends on arg n's value.
    define_args = alloc_obj();
    next_args = define_args;
    form(define_args) = CONS_FORM;
    car(define_args) = car(req_params);
    cdr(define_args) = NULL;
    mark_obj(define_args);
    cdr(define_args) = alloc_obj();
    form(cdr(define_args)) = CONS_FORM;
    cadr(define_args) = car(args);
    cddr(define_args) = nil_ptr;
    push();

    req_params = cdr(req_params);
    args = cdr(args);
  }

  return;
}

LISP_OBJ_PTR apply_derived(LISP_OBJ_PTR func, LISP_OBJ_PTR args) {
  LISP_OBJ_PTR params = func->value.proc.req_params;

  current_code = OP_BEGIN;
  current_env = alloc_environment();
  current_env->in_use = TRUE;
  current_env->enclosing_env = func->value.proc.env;
  current_args = proc_body(func);

  while (params != nil_ptr && args != nil_ptr) {
    enter_symbol(current_env, symbol_value(car(params)), car(args));
    params = cdr(params);
    args = cdr(args);
  }

  return NULL;
}

LISP_OBJ_PTR apply_begin() {
  if (!is_pair(current_args)) {
    current_res = nil_ptr;
    return nil_ptr;
  }

  if (cdr(current_args) != nil_ptr) {
    next_ret = NULL;
    next_code = OP_EVAL;
    next_env = current_env;
    next_args = car(current_args);
    current_args = cdr(current_args);
    push();
    return NULL;
  }

  current_code = OP_EVAL;
  current_args = car(current_args);
  return NULL;
}

LISP_OBJ_PTR apply_let() {
  if (!is_pair(current_args)) {
    current_res = nil_ptr;
    return nil_ptr;
  }

  // we have to be a bit careful here, we probably don't want to fully copy all
  // the args to let, but we don't want to disturb the passed in lists, if we
  // can avoid it.
  LISP_OBJ_PTR to_def = car(current_args);
  ENVIRONMENT_PTR new_env = alloc_environment();
  new_env->in_use = TRUE;
  new_env->enclosing_env = current_env;

  current_code = OP_BEGIN;
  current_env = new_env;
  current_args = cdr(current_args);
  while (is_pair(to_def)) {
    if (!is_pair(car(to_def))) {
      error_msg("ERROR -- pair expected in let binding");
      return NULL;
    }
    next_ret = NULL;
    next_code = OP_DEFINE;
    next_env = new_env;
    next_args = copy_list(car(to_def), alloc_obj());
    push();
    next_ret = &cadr(current_args);
    next_code = OP_EVAL;
    next_env = new_env;
    next_args = cadar(to_def);
    push();
    to_def = cdr(to_def);
  }

  return NULL;
}

LISP_OBJ_PTR apply_if() {
  if (!is_pair(current_args)) {
    current_res = nil_ptr;
    return nil_ptr;
  }
  // the first value of args should be a boolean that will tell us what to do
  if (is_true(car(current_args))) {
    current_code = OP_EVAL;
    current_args = cadr(current_args);
  } else if (cddr(current_args) != nil_ptr) {
    current_code = OP_EVAL;
    current_args = caddr(current_args);
  } else {
    current_res = nil_ptr;
    return nil_ptr;
  }

  return NULL;
}

LISP_OBJ_PTR apply_or() {
  if (is_null(current_args))
    return false_ptr;
  else if (is_true(car(current_args)))
    return true_ptr;
  // no need to alloc another frame here.
  else if (is_null(cdr(current_args)))
    return false_ptr;

  // we need to eval or on the tail
  current_code = OP_OR;
  current_args = cdr(current_args);
  eval_args(cdr(current_args));
  return NULL;
}

LISP_OBJ_PTR apply_and() {
  if (is_null(current_args))
    return true_ptr;
  else if (!is_true(car(current_args)))
    return false_ptr;
  else if (is_null(cdr(current_args)))
    return true_ptr;

  // we need to eval and on the tail
  current_code = OP_AND;
  current_args = cdr(current_args);
  eval_args(current_args);
  return NULL;
}

int main() {
  init_char_table();
  init_memory();
  init_global_env();
  eval_cycle();

  return 0;
}
