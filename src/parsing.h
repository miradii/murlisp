#include "../include/mpc.h"

/* an Enum type representing possible lval types */
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR };

/* create enumeration of possible error types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* declare new lval struct */
typedef struct lval {
  int type;
  long num;
  /* error and symbol types have some string data */
  char *err;
  char *sym;
  /* Count and pointer to a list of "lval*" */
  int count;
  struct lval **cell;
} lval;

/* Create a pointer to a new number type lval */
lval *lval_num(long x);

/* Create a pointer to a new error type lval*/
lval *lval_err(char *m);

/* Construct a pointer to a symbol type lval */
lval *lval_sym(char *s);

/* Construct a pointer to a sexpr type lval */
lval *lval_sexpr(void);

lval *lval_read(mpc_ast_t *t);

/* Deconstructor for lval type */
void lval_del(lval *v);

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
lval *lval_pop(lval *v, int i);

lval *lval_eval(lval *v);

lval *lval_take(lval *v, int i);

/* this evaluates the operation in an expression list */
lval *builtin_op(lval *, char *);
