/*************************/
/* STRUCTURES (Header)   */
/* FILE:    structures.h */
/* MODULE:  structures   */
/*************************/

#include "common.h"
#include "op_codes.h"

// lots of macros for dealing with structures.
#define car(x)              (x->value.cons.car)
#define cdr(x)              (x->value.cons.cdr)
#define caar(x)             car(car(x))
#define cadr(x)             car(cdr(x))
#define cdar(x)             cdr(car(x))
#define cddr(x)             cdr(cdr(x))
#define caaar(x)            car(car(car(x)))
#define cdaar(x)            cdr(car(car(x)))
#define cadar(x)            car(cdr(car(x)))
#define caadr(x)            car(car(cdr(x)))
#define cddar(x)            cdr(cdr(car(x)))
#define cdadr(x)            cdr(car(cdr(x)))
#define caddr(x)            car(cdr(cdr(x)))
#define cdddr(x)            cdr(cdr(cdr(x)))
#define is_pair(x)          (x->form == CONS_FORM)
#define is_symbol(x)        (x->form == SYMBOL_FORM)
#define is_proc(x)          (x->form == PROCEDURE_FORM)
#define is_bool(x)          (x->form == BOOLEAN_FORM)
#define is_string(x)        (x->form == STRING_FORM)
#define is_char(x)          (x->form == CHAR_FORM)
#define is_int(x)           (x->form == INT_FORM)
#define is_float(x)         (x->form == FLOAT_FORM)
#define is_number(x)        (x->form == INT_FORM || x->form == FLOAT_FORM)
#define is_null(x)          (x == nil_ptr)
#define is_true(x)          ((x->form == BOOLEAN_FORM && x->value.atom.bool_value)\
                             || x->form != BOOLEAN_FORM)
#define form(x)             (x->form)
#define int_value(x)        (x->value.atom.int_value)
#define float_value(x)      (x->value.atom.float_value)
#define char_value(x)       (x->value.atom.char_value)
#define string_value(x)     (x->value.atom.string_value)
#define bool_value(x)       (x->value.atom.bool_value)
#define symbol_value(x)     (x->value.atom.symbol_value)
#define proc_value(x)       (x->value.proc)
#define cons_value(x)       (x->value.cons)
#define proc_env(x)         (x->value.proc.env)
#define is_primitive(x)     (x->value.proc.type == PRIMITIVE)
#define is_derived(x)       (x->value.proc.type == DERIVED)
#define proc_type(x)        (x->value.proc.type)
#define proc_code(x)        (x->value.proc.primitive_code)
#define proc_body(x)        (x->value.proc.body)
#define proc_reqparams(x)   (x->value.proc.req_params)
#define proc_optparams(x)   (x->value.proc.opt_params)
#define proc_restparams(x)  (x->value.proc.rest_params)


// confirm these work
// these have all been implemented
#define current_ret         (current_frame->return_ptr)
#define current_res         (current_frame->result_ptr)
#define current_code        (current_frame->op_code)
#define current_func        (current_frame->func_ptr)
#define current_env         (current_frame->env)
#define current_args        (current_frame->args)
#define next_ret            (next_frame->return_ptr)
#define next_res            (next_frame->result_ptr)
#define next_code           (next_frame->op_code)
#define next_func           (next_frame->func_ptr)
#define next_env            (next_frame->env)
#define next_args           (next_frame->args)

#define frame_ret(x)      (x->return_ptr)
#define frame_res(x)      (x->result_ptr)
#define frame_code(x)     (x->op_code)
#define frame_func(x)     (x->func_ptr)
#define frame_env(x)      (x->env)
#define frame_args(x)     (x->args)

#define cons_hack(x)      (proc_code(x) == OP_CAR ||\
                           proc_code(x) == OP_CDR ||\
                           proc_code(x) == OP_CONS)


typedef enum {
  FALSE, TRUE,
} BOOLEAN;

typedef enum {
  NO_FORM,        // indicates that it's not being used.
  INT_FORM,
  FLOAT_FORM,
  CHAR_FORM,
  STRING_FORM,
  BOOLEAN_FORM,
  SYMBOL_FORM,
  CONS_FORM,
  PROCEDURE_FORM,
} OBJ_FORM;

typedef struct symtab_node {
  struct symtab_node *left, *right;            // pointers to subtrees
  char               name[text_max_id_length]; // name string
  struct lisp_obj    *obj;                      // value struct
  BOOLEAN            mark;                     // gc mark
} SYMTAB_NODE, *SYMTAB_NODE_PTR;

typedef struct environment {
  BOOLEAN            mark;           // gc mark
  BOOLEAN            in_use;         // let's us know if this is currently in use for allocation
  SYMTAB_NODE_PTR    root;           // root of the symbol table
  struct environment *enclosing_env; // the parent environment
} ENVIRONMENT, *ENVIRONMENT_PTR;


typedef struct lisp_obj {
  short mark; // gc mark

  OBJ_FORM form;

  union {
    union {
      int int_value;
      float float_value;
      char char_value;
      char *string_value;
      BOOLEAN bool_value;
      char *symbol_value;
    } atom;

    struct {
      enum {
        PRIMITIVE,
        DERIVED,
      } type;
      ENVIRONMENT_PTR   env;
      OP_CODE primitive_code;
      struct lisp_obj   *req_params;
      struct lisp_obj   *opt_params;
      struct lisp_obj   *rest_params;
      struct lisp_obj   *body;
    } proc;

    struct {
      struct lisp_obj *car;
      struct lisp_obj *cdr;
    } cons;
  } value;
} LISP_OBJ, *LISP_OBJ_PTR;


typedef struct stack_frame {
  LISP_OBJ_PTR    *return_ptr;      // where to store the result of the evaluation
  LISP_OBJ_PTR    result_ptr;       // the result of the evaluation
  OP_CODE         op_code;          // for built-in procedures
  LISP_OBJ_PTR    func_ptr;         // testing to see if this helps things...
  ENVIRONMENT_PTR env;              // the environment for the evaluation
  LISP_OBJ_PTR    args;             // the arguments for the op. repurposing the lisp objects
} STACK_FRAME, *STACK_FRAME_PTR;


/**************/
/* Prototypes */
/**************/
LISP_OBJ_PTR alloc_obj();
void free_obj();

// Searches the given symbol table for the given name string, crawling up the
// environment trees.
LISP_OBJ_PTR search_environment(ENVIRONMENT_PTR ep, char *name);

// Creates a new environment with the given parent environment.
ENVIRONMENT_PTR create_environment(ENVIRONMENT_PTR parent);

// Enters a symbol into the environment and links it to the given lisp object.
void enter_symbol(ENVIRONMENT_PTR ep, char *name, LISP_OBJ_PTR op);

SYMTAB_NODE_PTR create_symtab_node_ptr();

ENVIRONMENT_PTR alloc_environment();
void free_env(ENVIRONMENT_PTR envp);
void free_symtab(SYMTAB_NODE_PTR stp);


void pop();
void push();


// LISP_OBJ_PTR car(LISP_OBJ_PTR objp);
// LISP_OBJ_PTR cdr(LISP_OBJ_PTR objp);
LISP_OBJ_PTR cons(LISP_OBJ_PTR x, LISP_OBJ_PTR y);
LISP_OBJ_PTR set_car(LISP_OBJ_PTR cons_obj, LISP_OBJ_PTR to_set);
LISP_OBJ_PTR set_cdr(LISP_OBJ_PTR cons_obj, LISP_OBJ_PTR to_set);
void mut_reverse(LISP_OBJ_PTR *objp);
LISP_OBJ_PTR reverse(LISP_OBJ_PTR objp);
BOOLEAN is_special(LISP_OBJ_PTR objp);
void make_primitive(char *name, OP_CODE op_code);
BOOLEAN check_type(LISP_OBJ_PTR objp, OBJ_FORM form);
BOOLEAN check_all_number(LISP_OBJ_PTR list);
LISP_OBJ_PTR copy_list(LISP_OBJ_PTR from, LISP_OBJ_PTR into);
int list_length(LISP_OBJ_PTR list);

void gc();
void mark_env(ENVIRONMENT_PTR envp);
void mark_symtab(SYMTAB_NODE_PTR nodep);
void mark_stack();
void mark_obj(LISP_OBJ_PTR objp);
void sweep_envpool();
void sweep_objpool();
void unmark_symtab(SYMTAB_NODE_PTR nodep);
void unmark_env(ENVIRONMENT_PTR envp);
