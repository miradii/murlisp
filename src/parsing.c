#include "../include/mpc.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <string.h>
static char buffer[2048];

char *readline(char *prompt) {
  fputs(prompt, stdout);
  fgets(input, stdin);

  char *cpy = malloc(strlen(buffer) + 1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy) - 1] = '\0';
  return cpy;
}

void add_history(char *unused){};
#else
#include <editline/readline.h>
#endif
long eval_op(long, char *, long);
long eval(mpc_ast_t *t);
int main() {

  /* Defining some parsers */
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Operator = mpc_new("operator");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Murlisp = mpc_new("murlisp");

  mpca_lang(MPCA_LANG_DEFAULT, "                                              \
number   : /-?[0-9]+/;                          \
operator : '+' | '-' | '*' | '/';                \
expr     : <number> | '(' <operator> <expr>+ ')'; \
murlisp  : /^/<operator> <expr> + /$/;              ",
            Number, Operator, Expr, Murlisp);

  puts("Murlisp version 0.0.0.1");
  puts("Press Ctrl+c to exit");

  while (1) {
    // output our prompt
    char *input = readline("murlisp >");
    /* add history to input */
    add_history(input);

    /* Attempt to parse the user input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Murlisp, &r)) {
      /* On success print the AST */
      long result = eval(r.output);
      printf("%li\n", result);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  mpc_cleanup(4, Number, Operator, Expr, Murlisp);
  return 0;
}

/* a recursive function to evaluate the syntax tree
 * the base  case is a number cause if a node i a number it will not have any
 * children if a node is tagged expr we need to look at it's second child for
 * the operator causes the first one is a '(' */

long eval(mpc_ast_t *t) {
  /* if tagged as a number return it directly (base case) */
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  /* the operator is always the second child */
  char *op = t->children[1]->contents;

  /* we store the third child in x */
  long x = eval(t->children[2]);

  /* iterate the remaining children and combining */
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  return x;
}

long eval_op(long x, char *op, long y) {
  if (strcmp(op, "+") == 0) {
    return x + y;
  }
  if (strcmp(op, "*") == 0) {
    return x * y;
  }
  if (strcmp(op, "-") == 0) {
    return x - y;
  }
  if (strcmp(op, "/") == 0) {
    return x / y;
  }
  return 0;
}
