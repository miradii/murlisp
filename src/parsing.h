#include "../include/mpc.h"

/* an Enum type representing possible lval types */
enum { LVAL_ERR, LVAL_NUM };

/* create enumeration of possible error types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* declare new lval struct */
typedef struct {
  int type;
  long num;
  int err;
} lval;

/* Create a new number type lval */
lval lval_num(long x);

/* Create a new Error type lval */
lval lval_err(int x);

/* Print an "lval" */
void lval_print(lval v);

/* Print and "lval" followed by a newline */
void lval_println(lval v);

/* recursive function evaluating a mathematical operation */
lval eval_op(lval, char *, lval);

/* recursive function evaluating an AST  */
lval eval(mpc_ast_t *t);
