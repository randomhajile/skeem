#include "primitives.h"
#include <stdio.h>
#include <string.h>

// globals
extern LISP_OBJ_PTR nil_ptr;
extern LISP_OBJ_PTR true_ptr;
extern LISP_OBJ_PTR false_ptr;
extern void print_lispobj();
extern FILE *out_stream;

// for dealing with arithmetic
LISP_OBJ_PTR add(LISP_OBJ_PTR args) {
  LISP_OBJ_PTR res = alloc_obj();
  int int_res = 0;
  float float_res = 0;
  BOOLEAN is_float = FALSE;

  while (args != nil_ptr) {
    // check to see if we should be adding floats
    if (is_float(car(args)) && !is_float) {
      float_res = int_res;
      is_float = TRUE;
    }
    // grab the proper number
    if (is_float(car(args))) {
      float_res += float_value(car(args));
    } else if (!is_float)
      int_res += int_value(car(args));
    else
      float_res += int_value(car(args));
    args = cdr(args);
  }

  if (is_float) {
    res->form = FLOAT_FORM;
    float_value(res) = float_res;
  }
  else {
    res->form = INT_FORM;
    int_value(res) = int_res;
  }

  return res;
}

LISP_OBJ_PTR sub(LISP_OBJ_PTR args) {
  LISP_OBJ_PTR res = alloc_obj();
  int int_res = 0;
  float float_res = 0;
  BOOLEAN is_float = FALSE;

  if (is_int(car(args)))
    int_res = int_value(car(args));
  else {
    float_res = float_value(car(args));
    is_float = TRUE;
  }
  args = cdr(args);
  if (args == nil_ptr) {
    // unary minus
    if (is_float) {
      form(res) = FLOAT_FORM;
      float_value(res) = - float_res;
    } else {
      form(res) = INT_FORM;
      int_value(res) = -int_res;
    }
    return res;
  }

  while (args != nil_ptr) {
    // check to see if we should be adding floats
    if (is_float(car(args)) && !is_float) {
      float_res = int_res;
      is_float = TRUE;
    }
    // grab the proper number
    if (is_float(car(args))) {
      float_res -= float_value(car(args));
    } else if (!is_float)
      int_res -= int_value(car(args));
    else
      float_res -= int_value(car(args));
    args = cdr(args);
  }

  if (is_float) {
    form(res) = FLOAT_FORM;
    float_value(res) = float_res;
  }
  else {
    form(res) = INT_FORM;
    int_value(res) = int_res;
  }

  return res;
}

LISP_OBJ_PTR mult(LISP_OBJ_PTR args) {
  LISP_OBJ_PTR res = alloc_obj();
  int int_res = 1;
  float float_res = 1;
  BOOLEAN is_float = FALSE;

  while (args != nil_ptr) {
    // check to see if we should be adding floats
    if (is_float(car(args)) && !is_float) {
      float_res = int_res;
      is_float = TRUE;
    }
    // grab the proper number
    if (is_float(car(args))) {
      float_res *= float_value(car(args));
    } else if (!is_float)
      int_res *= int_value(car(args));
    else
      float_res *= int_value(car(args));
    args = cdr(args);
  }

  if (is_float) {
    form(res) = FLOAT_FORM;
    float_value(res) = float_res;
  }
  else {
    form(res) = INT_FORM;
    int_value(res) = int_res;
  }

  return res;
}

LISP_OBJ_PTR divide(LISP_OBJ_PTR args) {
  LISP_OBJ_PTR res = alloc_obj();
  float float_res = 0;
  // for now, this always coerces to a float
  form(res) = FLOAT_FORM;

  if (is_float(car(args))) {
    float_res = float_value(car(args));
  } else
    float_res = int_value(car(args));
  args = cdr(args);

  if (args == nil_ptr)  {
    float_value(res) = 1 / float_res;
    return res;
  }

  // TODO: check for zero division
  while (args != nil_ptr) {
    if (is_float(car(args)))
      float_res /= float_value(car(args));
    else
      float_res /= int_value(car(args));
    args = cdr(args);
  }

  float_value(res) = float_res;
  return res;
}

LISP_OBJ_PTR define(ENVIRONMENT_PTR env, LISP_OBJ_PTR args) {
  char *var;
  LISP_OBJ_PTR to_def = car(args);
  LISP_OBJ_PTR val;
  LISP_OBJ_PTR fun, params, req_params, opt_params, rest_params;
  ENVIRONMENT_PTR fun_env;

  BOOLEAN optional = FALSE, rest = FALSE;
  switch(form(to_def)) {
  case SYMBOL_FORM:
    var = symbol_value(to_def);
    val = cadr(args);
    enter_symbol(env, var, val);
    return to_def;
  case CONS_FORM:
    var = symbol_value(car(to_def));
    val = cdr(args);
    params = cdr(to_def);
    req_params = params;
    opt_params = nil_ptr;
    rest_params = nil_ptr;
    // edge case
    if (is_pair(params) && !strcmp(symbol_value(car(params)), "&optional")) {
      req_params = nil_ptr;
      optional = TRUE;
      // giving this a try
      opt_params = cdr(params);
      cdr(params) = nil_ptr;
      params = opt_params;
    }
    else if (is_pair(params) && !strcmp(symbol_value(car(params)), "&rest")) {
      req_params = nil_ptr;
      rest = TRUE;
      rest_params = cadr(params);
    }
    while (params != nil_ptr && !optional && !rest) {
      if (is_pair(cdr(params)) && !strcmp(symbol_value(cadr(params)), "&optional")) {
        optional = TRUE;
        opt_params = cddr(params);
        cdr(params) = nil_ptr;
        params = opt_params;
        break;
      } else if (is_pair(cdr(params)) && !strcmp(symbol_value(cadr(params)), "&rest")) {
        rest = TRUE;
        rest_params = caddr(params);
        cdr(params) = nil_ptr;
        params = rest_params;
        break;
      }
      params = cdr(params);
    }
    while (optional && params != nil_ptr && !rest) {
      if (is_pair(cdr(params)) && !strcmp(symbol_value(cadr(params)), "&rest")) {
        rest = TRUE;
        rest_params = caddr(params);
        cdr(params) = nil_ptr;
        params = rest_params;
        break;
      }
      params = cdr(params);
    }
    fun = alloc_obj();
    form(fun) = PROCEDURE_FORM;
    proc_type(fun) = DERIVED;
    proc_env(fun) = env;
    proc_reqparams(fun) = req_params;
    proc_optparams(fun) = opt_params;
    proc_restparams(fun) = rest_params;
    proc_body(fun) = val;
    enter_symbol(env, var, fun);
    return car(to_def);
  }
}

LISP_OBJ_PTR set(ENVIRONMENT_PTR env, LISP_OBJ_PTR args) {
  LISP_OBJ_PTR sym = car(args);
  LISP_OBJ_PTR val = car(cdr(args));

  enter_symbol(env, symbol_value(sym), val);

  return val;
}

LISP_OBJ_PTR make_lambda(ENVIRONMENT_PTR env, LISP_OBJ_PTR args) {
  // this code is a bit wet, we should combine with the define function
  LISP_OBJ_PTR fun, params;
  params = car(args);
  fun = alloc_obj();
  form(fun) = PROCEDURE_FORM;
  proc_type(fun) = DERIVED;
  proc_env(fun) = env;
  proc_reqparams(fun) = params;
  proc_optparams(fun) = nil_ptr;
  proc_restparams(fun) = nil_ptr;
  proc_body(fun) = cdr(args);

  return fun;
}

LISP_OBJ_PTR less_than(LISP_OBJ_PTR args) {
  LISP_OBJ_PTR current = car(args);
  args = cdr(args);

  // TODO: make this a lot less ugly.
  while (args != nil_ptr) {
    switch (form(current)) {
    case INT_FORM:
      switch (form(car(args))) {
      case INT_FORM:
        if (int_value(current) >= int_value(car(args)))
          return false_ptr;
        break;
      case FLOAT_FORM:
        if (int_value(current) >= float_value(car(args)))
          return false_ptr;
        break;
      }
      break;
    case FLOAT_FORM:
      switch (form(car(args))) {
      case INT_FORM:
        if (float_value(current) >= int_value(car(args)))
          return false_ptr;
        break;
      case FLOAT_FORM:
        if (float_value(current) >= float_value(car(args)))
          return false_ptr;
        break;
      }
      break;
    }
    current = car(args);
    args = cdr(args);
  }

  return true_ptr;
}

LISP_OBJ_PTR greater_than(LISP_OBJ_PTR args) {
  
  LISP_OBJ_PTR current = car(args);
  args = cdr(args);

  // TODO: make this a lot less ugly.
  while (args != nil_ptr) {
    switch (form(current)) {
    case INT_FORM:
      switch (form(car(args))) {
      case INT_FORM:
        if (int_value(current) <= int_value(car(args)))
          return false_ptr;
        break;
      case FLOAT_FORM:
        if (int_value(current) <= float_value(car(args)))
          return false_ptr;
        break;
      }
      break;
    case FLOAT_FORM:
      switch (form(car(args))) {
      case INT_FORM:
        if (float_value(current) <= int_value(car(args)))
          return false_ptr;
        break;
      case FLOAT_FORM:
        if (float_value(current) <= float_value(car(args)))
          return false_ptr;
        break;
      }
      break;
    }
    current = car(args);
    args = cdr(args);
  }

  return true_ptr;
}

LISP_OBJ_PTR less_than_eq(LISP_OBJ_PTR args) {
  LISP_OBJ_PTR current = car(args);
  args = cdr(args);

  // TODO: make this a lot less ugly.
  while (args != nil_ptr) {
    switch (form(current)) {
    case INT_FORM:
      switch (form(car(args))) {
      case INT_FORM:
        if (int_value(current) > int_value(car(args)))
          return false_ptr;
        break;
      case FLOAT_FORM:
        if (int_value(current) > float_value(car(args)))
          return false_ptr;
        break;
      }
      break;
    case FLOAT_FORM:
      switch (form(car(args))) {
      case INT_FORM:
        if (float_value(current) > int_value(car(args)))
          return false_ptr;
        break;
      case FLOAT_FORM:
        if (float_value(current) > float_value(car(args)))
          return false_ptr;
        break;
      }
      break;
    }
    current = car(args);
    args = cdr(args);
  }

  return true_ptr;
}

LISP_OBJ_PTR greater_than_eq(LISP_OBJ_PTR args) {
  LISP_OBJ_PTR current = car(args);
  args = cdr(args);

  // TODO: make this a lot less ugly.
  while (args != nil_ptr) {
    switch (form(current)) {
    case INT_FORM:
      switch (form(car(args))) {
      case INT_FORM:
        if (int_value(current) < int_value(car(args)))
          return false_ptr;
        break;
      case FLOAT_FORM:
        if (int_value(current) < float_value(car(args)))
          return false_ptr;
        break;
      }
      break;
    case FLOAT_FORM:
      switch (form(car(args))) {
      case INT_FORM:
        if (float_value(current) < int_value(car(args)))
          return false_ptr;
        break;
      case FLOAT_FORM:
        if (float_value(current) < float_value(car(args)))
          return false_ptr;
        break;
      }
      break;
    }
    current = car(args);
    args = cdr(args);
  }

  return true_ptr;
}

LISP_OBJ_PTR eq(LISP_OBJ_PTR args) {
  LISP_OBJ_PTR current = car(args);
  LISP_OBJ_PTR to_test;
  BOOLEAN res = TRUE;

  args = cdr(args);
  while (args != nil_ptr) {
    to_test = car(args);
    if (is_int(current) && is_int(to_test))
      res = (int_value(current) == int_value(to_test));
    else if (is_int(current) && is_float(to_test))
      res = (int_value(current) == float_value(to_test));
    else if (is_float(current) && is_int(to_test))
      res = (float_value(current) == int_value(to_test));
    else
      res = (float_value(current) == float_value(to_test));

    if (!res) {
      return false_ptr;
    }
    current = to_test;
    args = cdr(args);
  }

  return true_ptr;
}


void display(LISP_OBJ_PTR objp) {
  switch (objp->form) {
  case INT_FORM:
    fprintf(out_stream, "%d", int_value(objp));
    break;
  case FLOAT_FORM:
    fprintf(out_stream, "%g", float_value(objp));
    break;
  case CHAR_FORM:
    fprintf(out_stream, "%c", char_value(objp));
    break;
  case STRING_FORM:
    fprintf(out_stream, "%s", string_value(objp));
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
        printf(" . ");
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
