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

  /* Basic */
  long num;
  char *err;
  char *sym;

  /* Function */
  lbuiltin builtin;
  lenv *env;
  lval *formals;
  lval *body;

  /* Expression */
  int count;
  struct lval **cell;
};

/* struct representing the environment */
struct lenv {
  lenv *par;
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

/* a function for copying environments */
lenv *lenv_copy(lenv *e);

/* a function to add a variable to the global envrionment */
void lenv_def(lenv *e, lval *k, lval *v);

/* Create a pointer to a new number type lval */
lval *lval_num(long x);

/* Create a pointer to a new error type lval*/
lval *lval_err(char *fmt, ...);

/* Construct a pointer to a symbol type lval */
lval *lval_sym(char *s);

/* Construct a pointer to a sexpr type lval */
lval *lval_sexpr(void);

/* Construct a pointer to a func type lval */
lval *lval_func(lbuiltin);

/* Constructor for user defined lval functions */
lval *lval_lambda(lval *formals, lval *body);

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

char *ltype_name(int t);

/* this evaluates the operation in an expression list */
lval *builtin_op(lenv *e, lval *, char *);

/* Constructor for qxpression type lval */
lval *lval_qexpr(void);

lval *lval_join(lenv *e, lval *x, lval *y);

int lval_eq(lval *x, lval *y);

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
lval *builtin_lambda(lenv *e, lval *a);
lval *lval_qexpr(void);
lval *builtin_def(lenv *e, lval *a);
lval *builtin_put(lenv *e, lval *a);
lval *builtin_var(lenv *e, lval *a, char *func);

/* add a function to an environment  */
void lenv_add_builtin(lenv *e, char *name, lbuiltin func);

/* add all the built in functions to an env */
void lenv_add_builtins(lenv *e);

/* Ordering */
lval *builtin_gt(lenv *e, lval *a);
lval *builtin_lt(lenv *e, lval *a);
lval *builtin_ge(lenv *e, lval *a);
lval *builtin_le(lenv *e, lval *a);
lval *builtin_e(lenv *e, lval *a);
lval *builtin_ord(lenv *e, lval *a, char *op);
lval *builtin_cmp(lenv *e, lval *a, char *op);
lval *builtin_eq(lenv *e, lval *a);
lval *builtin_ne(lenv *e, lval *a);
lval *builtin_if(lenv *e, lval *a);
