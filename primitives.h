#include "structures.h"

LISP_OBJ_PTR add(LISP_OBJ_PTR args);
LISP_OBJ_PTR sub(LISP_OBJ_PTR args);
LISP_OBJ_PTR mult(LISP_OBJ_PTR args);
LISP_OBJ_PTR divide(LISP_OBJ_PTR args);
LISP_OBJ_PTR define(ENVIRONMENT_PTR env, LISP_OBJ_PTR args);
LISP_OBJ_PTR set(ENVIRONMENT_PTR env, LISP_OBJ_PTR args);
LISP_OBJ_PTR less_than(LISP_OBJ_PTR args);
LISP_OBJ_PTR greater_than(LISP_OBJ_PTR args);
LISP_OBJ_PTR less_than_eq(LISP_OBJ_PTR args);
LISP_OBJ_PTR greater_than_eq(LISP_OBJ_PTR args);
LISP_OBJ_PTR make_lambda(ENVIRONMENT_PTR env, LISP_OBJ_PTR args);
LISP_OBJ_PTR eq(LISP_OBJ_PTR args);
LISP_OBJ_PTR eqv(LISP_OBJ_PTR args);
void display(LISP_OBJ_PTR args);
