#include "primitives.h"
#include "token.h"

/*************************/
/* Prototypes for tokens */
/*************************/

void init_char_table();
void token();
void skip_whitespace();
char get_char();
void put_back();
void skip_comment();
void get_atom();
void get_s_char();
void get_string();
void get_sym();
void get_number();


/**************************/
/* Prototypes for reading */
/**************************/

LISP_OBJ_PTR read(LISP_OBJ_PTR read_to);
LISP_OBJ_PTR read_sexp(LISP_OBJ_PTR read_to);
LISP_OBJ_PTR read_list();
BOOLEAN init_memory();
BOOLEAN init_global_env();


/*****************************/
/* Prototypes for evaluation */
/*****************************/

void eval_cycle();
void read_evaluation();
void eval_stack();
void eval_current_frame();
void error_msg(char *msg);
void error();
LISP_OBJ_PTR eval_lispobj(LISP_OBJ_PTR objp);
LISP_OBJ_PTR apply_func(LISP_OBJ_PTR objp);
LISP_OBJ_PTR apply_derived(LISP_OBJ_PTR func, LISP_OBJ_PTR args);
void eval_funargs(ENVIRONMENT_PTR env, ENVIRONMENT_PTR eval_env, LISP_OBJ_PTR params, LISP_OBJ_PTR args);
void eval_args(LISP_OBJ_PTR args);
LISP_OBJ_PTR apply_begin();
LISP_OBJ_PTR apply_if();
LISP_OBJ_PTR apply_let();
LISP_OBJ_PTR apply_or();
LISP_OBJ_PTR apply_and();
LISP_OBJ_PTR apply_cond();
void bind_funargs(ENVIRONMENT_PTR env, LISP_OBJ_PTR func, LISP_OBJ_PTR args);

/**************************/
/* Prototypes for testing */
/**************************/
void print_token(TOKEN token);
void print_lispobj(LISP_OBJ_PTR objp);
