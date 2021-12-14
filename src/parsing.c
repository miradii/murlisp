#include "parsing.h"
#include "../include/mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* here is a macro for error checking */
#define LASSERT(args, cond, err)                                               \
  if (!(cond)) {                                                               \
    lval_del(args);                                                            \
    return lval_err(err);                                                      \
  }
#ifdef _WIN32

static char buffer[2048];

char *readline(char *prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char *cpy = malloc(strlen(buffer) + 1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy) - 1] = '\0';
  return cpy;
}

void add_history(char *unused) {}

#else
#include <editline/readline.h>
#endif

int main() {

  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Symbol = mpc_new("symbol");
  mpc_parser_t *Sexpr = mpc_new("sexpr");
  mpc_parser_t *Qexpr = mpc_new("qexpr");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Murlisp = mpc_new("murlisp");
  mpca_lang(MPCA_LANG_DEFAULT,
            "                                                     \
      number : /-?[0-9]+/ ;                               \
      symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;         \
      sexpr  : '(' <expr>* ')' ;                          \
      qexpr  : '{' <expr>* '}' ;                          \
      expr   : <number> | <symbol> | <sexpr> | <qexpr> ;  \
      murlisp  : /^/ <expr>* /$/ ;                          \
    ",
            Number, Symbol, Sexpr, Qexpr, Expr, Murlisp);
  puts("murlisp Version 0.0.0.0.6");
  puts("Press Ctrl+c to Exit\n");

  lenv *e = lenv_new();
  lenv_add_builtins(e);
  while (1) {

    char *input = readline("Murlisp> ");
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Murlisp, &r)) {
      lval *x = lval_eval(e, lval_read(r.output));
      lval_println(x);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }
  lenv_del(e);
  mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Murlisp);

  return 0;
}

/* constructor for lenv type */
lenv *lenv_new(void) {
  lenv *e = malloc(sizeof(lenv));
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}

/* decontructor for lenv */
void lenv_del(lenv *e) {
  for (int i = 0; i < e->count; i++) {
    free(e->syms[i]);
    lval_del(e->vals[i]);
  }
  free(e->syms);
  free(e->vals);
  free(e);
}

/* a function for getting variables from an env */
lval *lenv_get(lenv *e, lval *v) {
  /* iterate over all items in the environment */
  for (int i = 0; i < e->count; i++) {
    if (strcmp(e->syms[i], v->sym) == 0) {
      return lval_copy(e->vals[i]);
    }
  }

  // if no symbol found return an error
  return lval_err("unbound symbols");
}

/* a function for putting variables in an env */
void lenv_put(lenv *e, lval *k, lval *v) {
  /* iterate over all symbols if the symbol alreay exists replace it with new
   * value else add the new value */
  for (int i = 0; i < e->count; i++) {
    if (strcmp(e->syms[i], k->sym) == 0) {
      lval_del(e->vals[i]);
      e->vals[i] = lval_copy(v);
      return;
    }
  }

  /* if the symbols is not found we allocate memory for the new symbol an place
   * it in the environment */
  e->count++;
  e->vals = realloc(e->vals, sizeof(lval *) * e->count);
  e->syms = realloc(e->syms, sizeof(char *) * e->count);

  e->vals[e->count - 1] = lval_copy(v);
  e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
  strcpy(e->syms[e->count - 1], k->sym);
}

/* Construct a pointer to a new Number lval */
lval *lval_num(long x) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

/* Construct a pointer to a new Error lval */
lval *lval_err(char *m) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
  return v;
}

/* Construct a pointer to a new Symbol lval */
lval *lval_sym(char *s) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

/* A pointer to a new empty Sexpr lval */
lval *lval_sexpr(void) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

/* Construct a pointer to a func type lval */
lval *lval_func(lbuiltin func) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->fun = func;
  return v;
}

void lval_del(lval *v) {

  switch (v->type) {
  /* Do nothing special for number & func type */
  case LVAL_FUN:
  case LVAL_NUM:
    break;

    /* For Err or Sym free the string data */
  case LVAL_QEXPR:
  case LVAL_ERR:
    free(v->err);
    break;
  case LVAL_SYM:
    free(v->sym);
    break;

  /* If Sexpr then delete all elements inside */
  case LVAL_SEXPR:
    for (int i = 0; i < v->count; i++) {
      lval_del(v->cell[i]);
    }
    /* Also free the memory allocated to contain the pointers */
    free(v->cell);
    break;
  }

  /* Free the memory allocated for the "lval" struct itself */
  free(v);
}

lval *lval_add(lval *v, lval *x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval *) * v->count);
  v->cell[v->count - 1] = x;
  return v;
}

lval *lval_pop(lval *v, int i) {
  /* Find the item at "i" */
  lval *x = v->cell[i];

  /* Shift memory after the item at "i" over the top */
  memmove(&v->cell[i], &v->cell[i + 1], sizeof(lval *) * (v->count - i - 1));

  /* Decrease the count of items in the list */
  v->count--;

  /* Reallocate the memory used */
  v->cell = realloc(v->cell, sizeof(lval *) * v->count);
  return x;
}

lval *lval_take(lval *v, int i) {
  lval *x = lval_pop(v, i);
  lval_del(v);
  return x;
}

void lval_expr_print(lval *v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {

    /* Print Value contained within */
    lval_print(v->cell[i]);

    /* Don't print trailing space if last element */
    if (i != (v->count - 1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void lval_print(lval *v) {
  switch (v->type) {
  case LVAL_NUM:
    printf("%li", v->num);
    break;
  case LVAL_ERR:
    printf("Error: %s", v->err);
    break;
  case LVAL_SYM:
    printf("%s", v->sym);
    break;
  case LVAL_SEXPR:
    lval_expr_print(v, '(', ')');
    break;
  case LVAL_QEXPR:
    lval_expr_print(v, '{', '}');
    break;
  case LVAL_FUN:
    printf("<function>");
    break;
  }
}

void lval_println(lval *v) {
  lval_print(v);
  putchar('\n');
}

/* a function for copying lvals. it is used for copying things in and out of the
 * environment */
lval *lval_copy(lval *v) {
  lval *x = malloc(sizeof(lval));
  x->type = v->type;

  switch (v->type) {
  case LVAL_NUM:
    x->num = v->num;
    break;
  case LVAL_FUN:
    x->fun = v->fun;
    break;
  case LVAL_ERR:
    x->err = malloc(sizeof(v->err) + 1);
    strcpy(x->err, v->err);
    break;
  case LVAL_SYM:
    x->sym = malloc(sizeof(v->sym) + 1);
    strcpy(x->sym, v->sym);
    break;
  case LVAL_QEXPR:
  case LVAL_SEXPR:
    x->count = v->count;
    x->cell = malloc(sizeof(lval *) * v->count);
    for (int i = 0; i < x->count; i++) {
      x->cell[i] = lval_copy(v->cell[i]);
    }
    break;
  }
  return x;
}

lval *builtin_op(lenv *e, lval *a, char *op) {

  /* Ensure all arguments are numbers */
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err("Cannot operate on non-number!");
    }
  }

  /* Pop the first element */
  lval *x = lval_pop(a, 0);

  /* If no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0) && a->count == 0) {
    x->num = -x->num;
  }

  /* While there are still elements remaining */
  while (a->count > 0) {

    /* Pop the next element */
    lval *y = lval_pop(a, 0);

    /* Perform operation */
    if (strcmp(op, "+") == 0) {
      x->num += y->num;
    }
    if (strcmp(op, "-") == 0) {
      x->num -= y->num;
    }
    if (strcmp(op, "*") == 0) {
      x->num *= y->num;
    }
    if (strcmp(op, "/") == 0) {
      if (y->num == 0) {
        lval_del(x);
        lval_del(y);
        x = lval_err("Division By Zero.");
        break;
      }
      x->num /= y->num;
    }

    /* Delete element now finished with */
    lval_del(y);
  }

  /* Delete input expression and return result */
  lval_del(a);
  return x;
}

lval *lval_eval_sexpr(lenv *e, lval *v) {

  /* Evaluate Children */
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(e, v->cell[i]);
  }

  /* Error Checking */
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) {
      return lval_take(v, i);
    }
  }

  /* Empty Expression */
  if (v->count == 0) {
    return v;
  }

  /* Single Expression */
  if (v->count == 1) {
    return lval_take(v, 0);
  }

  /* Ensure First Element is a function */
  lval *f = lval_pop(v, 0);
  if (f->type != LVAL_FUN) {
    lval_del(f);
    lval_del(v);
    return lval_err("first element is not a function");
  }

  /* Call builtin with operator */
  lval *result = f->fun(e, v);
  lval_del(f);
  return result;
}

lval *lval_eval(lenv *e, lval *v) {

  /* Symbol look up happens here */
  if (v->type == LVAL_SYM) {
    lval *x = lenv_get(e, v);
    lval_del(v);
    return x;
  }
  /* Evaluate Sexpressions */
  if (v->type == LVAL_SEXPR) {
    return lval_eval_sexpr(e, v);
  }
  /* All other lval types remain the same */
  return v;
}

lval *lval_read_num(mpc_ast_t *t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval *lval_read(mpc_ast_t *t) {

  /* If Symbol or Number return conversion to that type */
  if (strstr(t->tag, "number")) {
    return lval_read_num(t);
  }
  if (strstr(t->tag, "symbol")) {
    return lval_sym(t->contents);
  }

  /* If root (>) or sexpr then create empty list */
  lval *x = NULL;
  if (strcmp(t->tag, ">") == 0) {
    x = lval_sexpr();
  }
  if (strstr(t->tag, "sexpr")) {
    x = lval_sexpr();
  }
  if (strstr(t->tag, "qexpr")) {
    x = lval_qexpr();
  }
  /* Fill this list with any valid expression contained within */
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->contents, ")") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->contents, "{") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->contents, "}") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->tag, "regex") == 0) {
      continue;
    }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}

/* constructor for q expression type lval */
lval *lval_qexpr(void) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

lval *builtin_head(lenv *e, lval *a) {
  /* Check Error conditions */
  LASSERT(a, a->count == 1, "Function \"head\" passed too many arguments!");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'head' passed incorrect types!");
  LASSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}");

  /* Otherwise take first argument */
  lval *v = lval_take(a, 0);

  /* Delete all elements that are not head and return  */
  while (v->count > 1) {
    lval_del(lval_pop(v, 1));
  }
  return v;
}

lval *builtin_tail(lenv *e, lval *a) {
  LASSERT(a, a->count == 1, "Function \"head\" passed too many arguments!");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'head' passed incorrect types!");
  LASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}");

  /* take first argument */
  lval *v = lval_take(a, 0);

  /* delete frist element and return */
  lval_del(lval_pop(v, 0));

  return v;
}

lval *builtin_list(lenv *e, lval *a) {
  a->type = LVAL_QEXPR;
  return a;
}

lval *builtin_eval(lenv *e, lval *a) {
  LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments!");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'eval' passed incorrect types!");

  lval *x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

/* the join function can be passed more than one argument */
lval *builtin_join(lenv *e, lval *a) {
  for (int i = 0; i < a->count; i++) {
    LASSERT(a, a->cell[i]->type == LVAL_QEXPR,
            "Function 'eval' passed incorrect types!");
  }
  lval *x = lval_pop(a, 0);

  while (a->count) {
    x = lval_join(e, x, lval_pop(a, 0));
  }

  lval_del(a);
  return x;
}

lval *lval_join(lenv *e, lval *x, lval *y) {
  while (y->count) {
    x = lval_add(x, lval_pop(y, 0));
  }

  return x;
}

/* arithmetic built in functions */
lval *builtin_add(lenv *e, lval *a) { return builtin_op(e, a, "+"); }
lval *builtin_min(lenv *e, lval *a) { return builtin_op(e, a, "-"); }

lval *builtin_mul(lenv *e, lval *a) { return builtin_op(e, a, "*"); }

lval *builtin_div(lenv *e, lval *a) { return builtin_op(e, a, "/"); }

/* built in lookup */
lval *builtin(lenv *e, lval *a, char *func) {
  if (strcmp("list", func) == 0) {
    return builtin_list(e, a);
  }
  if (strcmp("head", func) == 0) {
    return builtin_head(e, a);
  }
  if (strcmp("tail", func) == 0) {
    return builtin_tail(e, a);
  }
  if (strcmp("join", func) == 0) {
    return builtin_join(e, a);
  }
  if (strcmp("eval", func) == 0) {
    return builtin_eval(e, a);
  }
  if (strcmp("+", func) == 0) {
    return builtin_add(e, a);
  }
  if (strcmp("-", func) == 0) {
    return builtin_min(e, a);
  }
  if (strcmp("*", func) == 0) {
    return builtin_mul(e, a);
  }
  if (strcmp("/", func) == 0) {
    return builtin_div(e, a);
  }

  lval_del(a);
  return lval_err("Unknown function!");
}

void lenv_add_builtin(lenv *e, char *name, lbuiltin func) {
  lval *k = lval_sym(name);
  lval *v = lval_func(func);
  lenv_put(e, k, v);
  lval_del(k);
  lval_del(v);
}

void lenv_add_builtins(lenv *e) {
  /* List Functions */
  lenv_add_builtin(e, "eval", builtin_eval);
  printf("%d", e->count);
  lenv_add_builtin(e, "join", builtin_join);
  printf("%d", e->count);
  lenv_add_builtin(e, "list", builtin_list);
  printf("%d", e->count);
  lenv_add_builtin(e, "head", builtin_head);
  printf("%d", e->count);
  lenv_add_builtin(e, "tail", builtin_tail);

  /* Arithmetic functions */
  lenv_add_builtin(e, "+", builtin_add);
  printf("%d", e->count);
  lenv_add_builtin(e, "-", builtin_min);
  printf("%d", e->count);
  lenv_add_builtin(e, "*", builtin_mul);
  printf("%d", e->count);
  lenv_add_builtin(e, "/", builtin_div);
}
