#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"

/***********/
/* Globals */
/***********/
extern ENVIRONMENT_PTR environment_pool[MAX_NUM_ENVS];
extern ENVIRONMENT_PTR global_env_ptr;
extern LISP_OBJ_PTR    object_pool[MAX_NUM_OBJS];
extern LISP_OBJ_PTR    nil_ptr, true_ptr, false_ptr;
extern STACK_FRAME     stack[MAX_NUM_FRAMES];
extern STACK_FRAME_PTR current_frame;
extern STACK_FRAME_PTR next_frame;


/************************/
/* Environment routines */
/************************/
LISP_OBJ_PTR search_environment(ENVIRONMENT_PTR ep, char *name) {
  int cmp;
  ENVIRONMENT_PTR envp = ep;
  SYMTAB_NODE_PTR np = ep->root;

  // Loop to check each node. Return if the node matches else continue search.
  while (TRUE) {
    // if we have hit NULL, then we need to go the the parent environment.
    // make sure we start with a nonNULL np.
    while (np == NULL) {
      if (envp->enclosing_env == NULL)
        return NULL;
      envp = envp->enclosing_env;
      np = envp->root;
    }
    cmp = strcmp(name, np->name);
    if (cmp == 0) {
      return np->obj;                    // we found the name
    }
    np = cmp < 0 ? np->left : np->right;  // we did not find the name
  }

  return NULL;
}


void enter_symbol(ENVIRONMENT_PTR ep, char *name, LISP_OBJ_PTR op) {
  // This will enter/update the symbol table with the new lisp object
  int cmp;
  SYMTAB_NODE_PTR new_nodep;
  SYMTAB_NODE_PTR np     = ep->root;
  SYMTAB_NODE_PTR parent = NULL;

  // first, we need to figure out where the object should live in the symbol
  // table for the given environment
  while (np != NULL) {
    cmp = strcmp(name, np->name);
    if (cmp == 0) { // we have found the symbol and just need to update the symbol table
      break;
    }
    else {
      parent = np;
      np = cmp < 0 ? np->left : np->right;
    }
  }

  // if np is NULL, then we need to create a new object and tie it into the parent.
  // otherwise, we just update np.
  // NOTE: We do not need to worry about the lisp object being lost to the ether,
  // NOTE: it still lives in the lisp_obj allocation pool and that will eventually
  // NOTE: be garbage collected.
  if (np == NULL) {
    new_nodep = create_symtab_node_ptr();
    strcpy(new_nodep->name, name);
    new_nodep->left = new_nodep->right = NULL;
    new_nodep->mark = FALSE;
    new_nodep->obj = op;
    if (parent != NULL)
      cmp < 0 ? (parent->left = new_nodep) : (parent->right = new_nodep);
    else
      ep->root = new_nodep;
  } else {
    // we need to set the information on the old object.
    np->obj = op;
  }
}


SYMTAB_NODE_PTR create_symtab_node_ptr() {
  SYMTAB_NODE_PTR new_nodep = malloc(sizeof(SYMTAB_NODE));
  if (new_nodep == NULL) {
    // TODO: I don't exactly know how to handle this situation.
    fprintf(stderr, "\nERROR -- Out of memory.\n");
    exit(0);
  }
  new_nodep->left = NULL;
  new_nodep->right = NULL;
  new_nodep->obj = NULL;
  new_nodep->mark = FALSE;

  return new_nodep;
}


ENVIRONMENT_PTR alloc_environment() {
  int i;
  // TODO: same comments as alloc_obj
  for (i = 0; i < MAX_NUM_ENVS; ++i) {
    if (environment_pool[i]->in_use == FALSE) {
      return environment_pool[i];
    }
  }
  // if we are here we need to gc
  gc();
  for (i = 0; i < MAX_NUM_ENVS; ++i) {
    if (environment_pool[i]->in_use == FALSE) {
      return environment_pool[i];
    }
  }
  // TODO: This is probably not how to do this.
  fprintf(stderr, "\nERROR -- Environment pool empty.\n");
  exit(1);

  return NULL;
}

void free_env(ENVIRONMENT_PTR envp) {
  // NOTE: when you free an environment, you need to free the symtab pointers.
  // NOTE: you do not need to free the objects, that's the gc's job. you only
  // NOTE: have to free the symtabs
  free_symtab(envp->root);
  envp->root = NULL;
  envp->in_use = FALSE;
  envp->mark = FALSE;
}

void free_symtab(SYMTAB_NODE_PTR stp) {
  if (stp->left != NULL)
    free_symtab(stp->left);
  if (stp->right != NULL)
    free_symtab(stp->right);
  free(stp);
}

void gc() {
  mark_env(global_env_ptr);
  mark_stack();
  sweep_envpool();
  sweep_objpool();
  global_env_ptr->mark = FALSE;
  unmark_symtab(global_env_ptr->root);
}

void sweep_envpool() {
  int i;

  for (i = 0; i < MAX_NUM_ENVS; ++i) {
    if (environment_pool[i]->mark || environment_pool[i]->in_use == FALSE) {
      unmark_env(environment_pool[i]);
      continue;
    }
    free_env(environment_pool[i]);
  }
}

void sweep_objpool() {
  int i;

  for (i = 0; i < MAX_NUM_OBJS; ++i) {
    if (object_pool[i]->mark || object_pool[i]->form == NO_FORM) {
      object_pool[i]->mark = FALSE;
      continue;
    }
    free_obj(object_pool[i]);
  }
}

void mark_env(ENVIRONMENT_PTR envp) {
  // starts at the global environment and marks all lisp objects/envs accessible
  // from there
  // note that the global env lives outside the env pool and will never be up
  // for collection
  if (envp == NULL || envp->mark)
    return;

  envp->mark = TRUE;
  mark_symtab(envp->root);
  mark_env(envp->enclosing_env);
}

void mark_symtab(SYMTAB_NODE_PTR nodep) {
  if (nodep == NULL || nodep->mark)
    return;
  nodep->mark = TRUE;
  mark_obj(nodep->obj);
  mark_symtab(nodep->left);
  mark_symtab(nodep->right);
}

void unmark_symtab(SYMTAB_NODE_PTR nodep) {
  if (nodep == NULL)
    return;
  nodep->mark = FALSE;
  unmark_symtab(nodep->left);
  unmark_symtab(nodep->right);
}

void unmark_env(ENVIRONMENT_PTR envp) {
  envp->mark = FALSE;
  unmark_symtab(envp->root);
}

void mark_stack() {
  STACK_FRAME_PTR frame = current_frame;

  // make sure we don't fall off the edge of the earth here...
  while (frame >= stack) {
    mark_env(frame->env);
    mark_obj(frame->args);
    mark_obj(frame->result_ptr);
    if (frame->return_ptr != NULL)
      mark_obj(*frame->return_ptr);
    --frame;
  }
}

void mark_obj(LISP_OBJ_PTR objp) {
  if (objp == NULL || is_null(objp) || objp->mark) {
    return;
  }

  objp->mark = TRUE;
  switch (objp->form) {
  case CONS_FORM:
    mark_obj(car(objp));
    mark_obj(cdr(objp));
    break;
  case PROCEDURE_FORM:
    mark_env(objp->value.proc.env);
    mark_obj(objp->value.proc.body);
    mark_obj(objp->value.proc.req_params);
    break;
  }
}


/************************/
/* Lisp object routines */
/************************/
LISP_OBJ_PTR alloc_obj() {
  int i;

  // TODO: Note that if we call alloc_obj(); alloc_obj(); without
  // TODO: setting the form first, this will cause problems, since we're using
  // TODO: the form as a way to signal use.
  // TODO: turn this into a function
  for (i = 0; i < MAX_NUM_OBJS; ++i) {
    if (object_pool[i]->form == NO_FORM) {
      return object_pool[i];
    }
  }
  // if we're here, then we need to gc
  gc();
  for (i = 0; i < MAX_NUM_OBJS; ++i) {
    if (object_pool[i]->form == NO_FORM) {
      return object_pool[i];
    }
  }
  // and if we're here, then we're out of memory.
  fprintf(stderr, "\nERROR -- Object pool empty.\n");
  exit(1);
  
  return NULL;
}

void free_obj(LISP_OBJ_PTR p) {
  if (p->form == STRING_FORM)
    free(p->value.atom.string_value);
  else if (p->form == SYMBOL_FORM)
    free(p->value.atom.symbol_value);
  else if (p == nil_ptr || p == true_ptr || p == false_ptr)
    return;

  p->form = NO_FORM;
  p->mark = FALSE;
}


/****************************/
/* Stack Frame manipulation */
/****************************/
// TODO: should probably come up with a more sophisticated stack, but this will
// TODO: do for now.
void push() {
  if (next_frame - stack == MAX_NUM_FRAMES + 1)
    fprintf(stderr, "\nERROR -- Stack Overflow.\n");
  else {
    next_frame->result_ptr = NULL;
    current_frame = next_frame++;
  }
}

void pop() {
  if (current_frame != stack) {
    if (current_frame->return_ptr != NULL)
      *current_frame->return_ptr = current_frame->result_ptr;
    current_frame->result_ptr = NULL;
    current_frame->return_ptr = NULL;
    current_frame->env = NULL;
    current_frame->args = NULL;
    --current_frame;
  }
  if (next_frame != stack)
    --next_frame;
}


/****************************/
/* Lisp Object Manipulation */
/****************************/

LISP_OBJ_PTR cons(LISP_OBJ_PTR x, LISP_OBJ_PTR y) {
  LISP_OBJ_PTR res = alloc_obj();
  res->form = CONS_FORM;
  res->value.cons.car = x;
  res->value.cons.cdr = y;

  return res;
}

LISP_OBJ_PTR set_car(LISP_OBJ_PTR cons_obj, LISP_OBJ_PTR to_set) {
  cons_obj->value.cons.car = to_set;

  return cons_obj;
}

LISP_OBJ_PTR set_cdr(LISP_OBJ_PTR cons_obj, LISP_OBJ_PTR to_set) {
  cons_obj->value.cons.cdr = to_set;

  return cons_obj;
}

void mut_reverse(LISP_OBJ_PTR *objp) {
  // NOTE: *objp will be reverse, but if anything points to a cell in the middle
  // NOTE: of the list, it will be ten kinds of jacked.
  LISP_OBJ_PTR rest, current;
  LISP_OBJ_PTR list = *objp;

  if (list == nil_ptr)
    return;

  rest = cdr(list);
  current = nil_ptr;

  while (rest != nil_ptr) {
    list->value.cons.cdr = current;
    current = list;
    list = rest;
    rest = cdr(rest);
  }

  list->value.cons.cdr = current;
  *objp = list;
}

LISP_OBJ_PTR reverse(LISP_OBJ_PTR objp) {
  LISP_OBJ_PTR res = alloc_obj();
  LISP_OBJ_PTR previous;

  if (objp == nil_ptr)
    return objp;

  res->form = CONS_FORM;
  res->value.cons.car = car(objp);
  res->value.cons.cdr = nil_ptr;
  previous = res;
  res = alloc_obj();
  objp = cdr(objp);
  while (objp != nil_ptr) {
    res = alloc_obj();
    res->form = CONS_FORM;
    res->value.cons.car = car(objp);
    res->value.cons.cdr = previous;
    previous = res;
    objp = cdr(objp);
  }

  return res;
}

BOOLEAN is_special(LISP_OBJ_PTR objp) {
  if (objp->form != PROCEDURE_FORM)
    return FALSE;
  else if (objp->value.proc.type == PRIMITIVE)
    return TRUE;
  else
    return FALSE;
}

void make_primitive(char *name, OP_CODE op_code) {
  LISP_OBJ_PTR func = alloc_obj();
  func->form = PROCEDURE_FORM;
  func->value.proc.type = PRIMITIVE;
  func->value.proc.primitive_code = op_code;
  func->value.proc.env = global_env_ptr;
  enter_symbol(global_env_ptr, name, func);
}

BOOLEAN check_type(LISP_OBJ_PTR objp, OBJ_FORM form) {
  return objp->form == form;
}

BOOLEAN check_all_number(LISP_OBJ_PTR list) {
  while (list != nil_ptr) {
    if (!check_type(car(list), INT_FORM) && !check_type(car(list), FLOAT_FORM))
      return FALSE;
    list = cdr(list);
  }
  return TRUE;
}

LISP_OBJ_PTR copy_list(LISP_OBJ_PTR from, LISP_OBJ_PTR into) {
  // it's up to you to make sure your list is actually a list, this will error
  // if it isn't
  if (from == nil_ptr)
    return nil_ptr;

  LISP_OBJ_PTR res = into;
  res->form = CONS_FORM;
  LISP_OBJ_PTR current = res;

  while (TRUE) {
    current->value.cons.car = car(from);
    from = cdr(from);
    if (!is_pair(from)) {
      current->value.cons.cdr = nil_ptr;
      break;
    }
    current->value.cons.cdr = nil_ptr;
    current->value.cons.cdr = alloc_obj();
    current = cdr(current);
    current->form = CONS_FORM;
  }

  return res;
}

int list_length(LISP_OBJ_PTR list) {
  // it's up to you to make sure your list is actually a list, this will error
  // if it isn't
  int res = 0;

  while (list != nil_ptr) {
    res += 1;
    list = cdr(list);
  }

  return res;
}
