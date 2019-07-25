// basic REPL loop that will build the foundation for the parser 

// compile command 
// gcc -std=c99 -Wall repl.c mpc.c -ledit -lm -o prompt

#include<stdio.h> 
#include<stdlib.h> 
#include<stdbool.h>
#include<math.h> 
#include "mpc.h"

// for windows users 
#ifdef _WIN32
#include <string.h>
#define MAXLINE 2048 

static char buffer[2048];

char* readline(char* prompt) {
	fputs(prompt, stdout); 
	fgets(buffer, MAXLINE, stdin); 
	char* in = malloc(strlen(buffer)+1); 
	strcpy(in, buffer); 
	cpy[strlen(in)-1] = '\0'; 	// chop newline
	return in; 
}

void add_history(char* reserve) {/* space filler */} 

#else 
#include <editline/readline.h>
#endif 

typedef struct sval {
	int type; 
	long num; 
	// error and symbol types are now bound to char* 
	char* err; 
	char* sym;
	// pointer to nested S-expr, and array len 
	struct sval** cell;
	int count; 
} sval; 

typedef enum {
	SVAL_NUM, 
	SVAL_ERR, 
	SVAL_SYM, 
	SVAL_SEXPR
} sValueType; 

typedef enum {
	SERR_ZERO_DIV, 
	SERR_BAD_OP, 
	SERR_BAD_NUM
} sErrType; 

// cf. §9 -- Refactoring for pointers 

// number type 
sval* sval_num(long x) {
	sval* v = malloc(sizeof(sval));  
	v->type = SVAL_NUM;
	v->num = x; 
	return v; 
}

// error type 
sval* sval_err(char* e) {
	sval* v = malloc(sizeof(sval)); 
	v->type = SVAL_ERR; 
	v->err = malloc(strlen(e)+1);
	strcpy(v->err, e); 
	return v; 
}

// symbol value 
sval* sval_sym(char* s) {
	sval* v = malloc(sizeof(sval)); 
	v->type = SVAL_SYM; 
	v->sym  = malloc(strlen(s)+1); // shouldn't this be a calloc??? 
	strcpy(v->sym, s); 
	return v; 
}

// S-expr value 
sval* sval_sexpr(void) {
	// just a container 
	sval* v = malloc(sizeof(sval)); 
	v->type = SVAL_SEXPR; 
	v->count = 0; 
	v->cell = NULL; 
	return v; 
}

// nests one S-Expr inside of another 
sval* sval_add(sval* v, sval* x) {
	v->count++; 
	v->cell = realloc(v->cell, sizeof(sval*) * v->count); 
	v->cell[v->count-1] = x; 
	return v; 
}

// reading from the AST -> creating sval* 
sval* sval_read_num(mpc_ast_t* t) {
	errno = 0; 
	long x = strtol(t->contents, NULL, 10); 
	return errno != ERANGE ? 
		sval_num(x) : sval_err("invalid number"); 
}

sval* sval_read(mpc_ast_t* t) {
	// debug printf("sval_read() "); 
	// if a symbol, return the conversion to that type 
	if (strstr(t->tag, "number")) { return sval_read_num(t); }
	if (strstr(t->tag, "symbol")) { return sval_sym(t->contents); }
	// if the parsed token is root or an S-Expr, then we need to create 
	// a container 
	sval* x = NULL; 
	if (strcmp(t->tag, ">") == 0) { x = sval_sexpr(); }
	if (strstr(t->tag, "sexpr")) 	{ x = sval_sexpr(); }

	// fill in the new S-Expr with any valid expressions 
	for (int i = 0; i < t->children_num; i++) {
		// weed through any meta-information 
		if (strcmp(t->children[i]->contents, "(") == 0) {continue;}
		if (strcmp(t->children[i]->contents, ")") == 0) {continue;}
		if (strcmp(t->children[i]->tag,  "regex") == 0) {continue;}
		x = sval_add(x, sval_read(t->children[i])); 
	}
	return x; 
}


// destructing svals 
void sval_free(sval* v) {
// debug printf("sval_free() "); 
	switch (v->type) {
		// number types have no additional memory to be freed
		case SVAL_NUM: 
			break; 
		case SVAL_ERR: free(v->err); break; 
		case SVAL_SYM: free(v->sym); break; 
		// an S-expr requires mo' freein' 
		case SVAL_SEXPR:
			for (int i = 0; i < v->count; i++) {
				sval_free(v->cell[i]); 
			}
			free(v->cell); 
			break; 
	}
	free(v); 	// free the structure itself, too. 
}


// helpers 

long findMin(long x, long y) {
	return (x > y) ? y : x; 
}

long findMax(long x, long y) {
	return (x > y) ? x : y; 
}

// print functions 
// need forward declarations to resolve dependencies 
void sval_expr_print(sval* v, char open, char close); 
void sval_print(sval* v); 

void sval_expr_print(sval* v, char open, char close) {
	putchar(open); 
	for (int i = 0; i < v->count; i++) {

		// print value of current sval 
		sval_print(v->cell[i]); 
		// don't print trailing spaces for the last element 
		if (i != (v->count-1)) {
			putchar(' '); 
		}
	}
	putchar(close); 

}

void sval_print(sval* v) {
	switch (v->type) {
		case SVAL_NUM: 		printf("%li", v->num); 			  break; 
		case SVAL_ERR: 		printf("Error: %s", v->err);  break; 
		case SVAL_SYM: 		printf("%s", v->sym); 			  break; 
		case SVAL_SEXPR:  sval_expr_print(v, '(', ')'); break; 
	}
}

void sval_println(sval* v) { sval_print(v); putchar('\n'); }

// // // // // // // // // // 
// Evaluating Expressions  //
// // // // // // // // // // 

sval* sval_eval(sval* v); 
sval* sval_pop(sval* v, int i); 
sval* sval_take(sval* v, int i); 
sval* builtin_op(sval* v, char* op); 

sval* sval_eval_sexpr(sval* v) {
	// debug printf("sval_eval_sexpr() "); 
	// evaluate from the inside out 
	for (int i = 0; i < v->count; i++) {
		v->cell[i] = sval_eval(v->cell[i]); 
	}

	// check for errors 
	for (int i = 0; i < v->count; i++) {
		if (v->cell[i]->type == SVAL_ERR) { return sval_take(v, i); }
	}

	// empty expression case 
	if (v->count == 0) { return v; }
	// single expression case
	if (v->count == 1) { return sval_take(v, 0); }

	// ensure that the first element is a symbol 
	sval* f = sval_pop(v, 0); 
	if (f->type != SVAL_SYM) {
		sval_free(f); sval_free(v); 
		// throw syntax error 
		return sval_err("S-expression does not start with a valid symbol."); 
	}

	// pass expression and arguments to builtin 
	sval* result = builtin_op(v, f->sym); 
	sval_free(f); 
	return result; 
}

sval* sval_eval(sval* v) {
	// evaluate S-expressions 
	if (v->type == SVAL_SEXPR) { return sval_eval_sexpr(v); }
	// all other types can be returned 
	return v; 
}

// pops an element from the list, leaving the remainder of the list in 
// tact. differs from sval_take, which deletes it all 
sval* sval_pop(sval* v, int i) {
	// debug printf("sval_pop() "); 
	// find the ith item 
	sval* x = v->cell[i]; 

	// shift memory after the item at 'i' over the top 
	memmove(&v->cell[i], &v->cell[i+1], 
		sizeof(sval*) * (v->count-i-1)); 

	// decrement the count of items in the list 
	v->count--; 

	// reallocate the used memory 
	v->cell = realloc(v->cell, sizeof(sval*) * v->count); 
	return x; 
}

// extracts an elment and then deletes the list it came from 
// perhaps useful when we hit bottom? 
sval* sval_take(sval* v, int i) {
	sval* x = v->cell[i]; 
	sval_free(v); 
	return x; 
}

sval* builtin_op(sval* a, char* op) {
	// debug printf("builtin_op() "); 
	// verify that input arguments are all numbers 
	for (int i = 0; i < a->count; i++) {
		if (a->cell[i]->type != SVAL_NUM) {
			sval_free(a); 
			return sval_err("Cannot operate on non-number arguments!"); 
		}
	}

	// pop the first element 
	sval* x = sval_pop(a, 0); 

	// if no args, and '-', perform unary negation 
	if ((strcmp(op, "-") == 0) && a->count == 0) {
		x->num = -1 * x->num; 
	}

	// while arguments remain 
	while (a->count > 0) {

		// pop the next argument 
		sval* y = sval_pop(a, 0); 

		// evaluate 
		if (strcmp(op, "+") == 0) { x->num += y->num; }
		if (strcmp(op, "-") == 0) { x->num -= y->num; }
		if (strcmp(op, "*") == 0) { x->num *= y->num; }
		if (strcmp(op, "^") == 0) { x->num = pow(x->num, y->num); }
		if (strcmp(op, "max") == 0) { x->num = findMax(x->num, y->num); }
		if (strcmp(op, "min") == 0) { x->num = findMin(x->num, y->num); }
		// following operations require nonzero operands 
		if (y->num == 0) { 
			sval_free(x); sval_free(y); 
			x = sval_err("Division by zero!"); break; 
		}
		if (strcmp(op, "/") == 0) { x->num = x->num / y->num; }
		if (strcmp(op, "%") == 0) { x->num = x->num % y->num; }

		sval_free(y); 
	}

	sval_free(a); 
	return x; 
}


int main(int argc, char* argv[]) {

// Create Parsers 
mpc_parser_t* Number   = mpc_new("number"); 
mpc_parser_t* Symbol   = mpc_new("symbol");
mpc_parser_t* Sexpr 	 = mpc_new("sexpr"); 
mpc_parser_t* Expr     = mpc_new("expr"); 
mpc_parser_t* Cue 		 = mpc_new("cue"); 

// Define Language 
mpca_lang(MPCA_LANG_DEFAULT, 
	"																			   							\
		number   : /-?[0-9]+/ ; 							 							\
		symbol   : '+' | '-' | '/' | '*' | '%' | '^' |			\
								/min/ | /max/ ; 												\
	  sexpr    : '(' <expr>* ')' ; 												\
		expr 		 : <number> | <symbol> | <sexpr>  ;					\
		cue      : /^/ <expr>* /$/ ; 												\
		", 
		Number, Symbol, Sexpr, Expr, Cue);   


	// print version and exit information 
	puts("Safiniea Version 0.0.1"); 
	puts("Ctrl-C to exit."); 

	// REPL loop 
	while (true) {

		// prompt 
		char* input = readline("\n∞ > "); 

		// add input to history  
		add_history(input); 

		// attempt to parse user input 
		mpc_result_t r; 
		if (mpc_parse("<stdin>", input, Cue, &r)) {
			// success 
			//mpc_ast_print(r.output);
			sval* x = sval_eval(sval_read(r.output)); 
			sval_println(x); 
			sval_free(x); 

			mpc_ast_delete(r.output); 
		} else {
			// err 
			mpc_err_print(r.error); 
			mpc_err_delete(r.error); 
		}

		// free retrieved input
		free(input); 


	}
	// undefine and clean up Parsers 
	mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Cue);
	return(EXIT_SUCCESS); 
}