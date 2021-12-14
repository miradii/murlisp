#include "../include/mpc.h"
#include <stdlib.h>

/* forward decelerations */
struct lenv;
struct lval;
typedef struct lenv lenv;
typedef struct lval lval;

/* function pointer type for environment */
typedef lval *(*lbuiltin)(lenv *, lval *);

/* an Enum type representing possible lval types */
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR, LVAL_FUN };

/* create enumeration of possible error types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* declare new lval struct */
struct lval {
  int type;
  long num;
  /* error and symbol types have some string data */
  char *err;
  char *sym;
  lbuiltin fun;
  /* Count and pointer to a list of "lval*" */
  int count;
  struct lval **cell;
};

/* struct representing the environment */
struct lenv {
  int count;
  char **syms;
  lval **vals;
};
/* constructor for lenv type */
lenv *lenv_new(void);

/* deconstructor for the lenv type */
void lenv_del(lenv *);

/* a function for putting variables in the environment */
void lenv_put(lenv *e, lval *k, lval *v);

/* a function for getting variables from an environment */
lval *lenv_get(lenv *e, lval *k);

/* Create a pointer to a new number type lval */
lval *lval_num(long x);

/* Create a pointer to a new error type lval*/
lval *lval_err(char *m);

/* Construct a pointer to a symbol type lval */
lval *lval_sym(char *s);

/* Construct a pointer to a sexpr type lval */
lval *lval_sexpr(void);

/* Construct a pointer to a func type lval */
lval *lval_func(lbuiltin);

lval *lval_read(mpc_ast_t *t);

/* Deconstructor for lval type */
void lval_del(lval *v);
lval *lval_copy(lval *v);

/* Print an "lval" */
void lval_print(lval *v);

/* Print lval expr */
void print_lval_expr(lval *, char, char);

/* Print and "lval" followed by a newline */
void lval_println(lval *v);

lval *lval_read_num(mpc_ast_t *t);

/* add an item to the expression list */
lval *lval_add(lval *v, lval *x);

/* remove an item from the expression list */
lval *lval_pop(lval *, int i);

lval *lval_eval(lenv *, lval *);

lval *lval_eval_sexpr(lenv *, lval *);

lval *lval_take(lval *v, int i);

/* this evaluates the operation in an expression list */
lval *builtin_op(lenv *e, lval *, char *);

/* Constructor for qxpression type lval */
lval *lval_qexpr(void);

lval *lval_join(lenv *e, lval *x, lval *y);

lval *builtin(lenv *e, lval *a, char *func);
lval *builtin_join(lenv *e, lval *a);
lval *builtin_eval(lenv *e, lval *a);
lval *builtin_list(lenv *e, lval *a);
lval *builtin_tail(lenv *e, lval *a);
lval *builtin_head(lenv *e, lval *a);
lval *builtin_add(lenv *e, lval *a);
lval *builtin_min(lenv *e, lval *a);
lval *builtin_mul(lenv *e, lval *a);
lval *builtin_div(lenv *e, lval *a);
lval *lval_qexpr(void);

/* add a function to an environment  */
void lenv_add_builtin(lenv *e, char *name, lbuiltin func);

/* add all the built in functions to an env */
void lenv_add_builtins(lenv *e);
