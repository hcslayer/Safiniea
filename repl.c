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

// forward declarations 
struct sval; 
struct env; 
typedef struct sval sval;
typedef struct env env; 
typedef sval*(*sbuiltin)(env*, sval*); 


typedef struct sval {
	int type; 
	long num; 
	sbuiltin fun; 
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
	SVAL_SEXPR,
	SVAL_QEXPR, 
	SVAL_FUN
} sValueType; 

typedef enum {
	SERR_ZERO_DIV, 
	SERR_BAD_OP, 
	SERR_BAD_NUM
} sErrType; 

char* stype_name(sValueType t){
	switch(t) {
		case SVAL_FUN: return "Function"; 
		case SVAL_NUM: return "Number"; 
		case SVAL_SYM: return "Symbol"; 
		case SVAL_ERR: return "ERror"; 
		case SVAL_SEXPR: return "S-Expression"; 
		case SVAL_QEXPR: return "Q-Expression"; 
		default: return "???"; 
	}
}

// cf. §9 -- Refactoring for pointers 

// number type 
sval* sval_num(long x) {
	sval* v = malloc(sizeof(sval));  
	v->type = SVAL_NUM;
	v->num = x; 
	return v; 
}

// error type 
sval* sval_err(char* fmt, ...) {
	sval* v = malloc(sizeof(sval)); 
	v->type = SVAL_ERR; 
	
	// create and initialize the va list 
	va_list va; 
	va_start(va, fmt); 

	// allocate 512 bytes 
	v->err = malloc(512); 

	// print error message with a max of 511 chars 
	vsnprintf(v->err, 511, fmt, va); 
	// allocate just enough space 
	v->err = realloc(v->err, strlen(v->err)+1);
	va_end(va); 

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

// Q-expr values
sval* sval_qexpr(void) {
	sval* v = malloc(sizeof(sval)); 
	v->type = SVAL_QEXPR;
	v->count = 0; 
	v->cell = NULL;
	return v; 
}

// Functional values 
sval* sval_fun(sbuiltin func) {
	sval* v = malloc(sizeof(sval)); 
	v->type = SVAL_FUN; 
	v->fun = func; 
	return v; 
}

struct env {
	int count; 
	char** syms; 
	sval** vals; 
}; 

env* env_new(void) {
	env* e = malloc(sizeof(env)); 
	e->count = 0; 
	e->syms = NULL; 
	e->vals = NULL; 
	return e; 
}

// free svals 
void sval_free(sval* v) {

	switch(v->type) {
		// do nothing special for numbers 
		case SVAL_NUM: break; 
		case SVAL_FUN: break; 

		// for err or sym, free string data 
		case SVAL_ERR: free(v->err); break; 
		case SVAL_SYM: free(v->sym); break; 

		// in the case of an S-Expr, free internal data 
		case SVAL_SEXPR: 
		case SVAL_QEXPR:
			for (int i = 0; i < v->count; i++) {
				sval_free(v->cell[i]);
			}
			free(v->cell); 
			break; 
	}
	// free the struct itself. 
	free(v); 
}

void env_del(env* e) {
	for (int i = 0; i < e->count; i++) {
		free(e->syms[i]); 
		sval_free(e->vals[i]); 
	}
	free(e->syms); 
	free(e->vals); 
	free(e);
}



// copy function to set up environments 
sval* sval_copy(sval* v) {
	sval* x = malloc(sizeof(sval)); 
	x->type = v->type; 

	switch(v->type) {
		case SVAL_FUN: x->fun = v->fun; break; 
		case SVAL_NUM: x->num = v->num; break; 
		// dynamic operations 
		case SVAL_ERR: 
			x->err = malloc(strlen(v->err)+1); 
			strcpy(x->err, v->err); 
			break;
		case SVAL_SYM: 
			x->sym = malloc(strlen(v->sym)+1); 
			strcpy(x->sym, v->sym); 
			break; 
		// lists require that we copy sub-expressions, too 
		case SVAL_SEXPR:
		case SVAL_QEXPR:
			x->count = v->count; 
			x->cell = malloc(sizeof(sval*) * x->count); 
			for (int i = 0; i < x->count; i++) {
				x->cell[i] = sval_copy(v->cell[i]); 
			}
			break; 
	}
	return x; 
}

// return environmental bindings
sval* env_get(env* e, sval* k) {
	for (int i = 0; i < e->count; i++) {
		if (strcmp(e->syms[i], k->sym) == 0) {
			return sval_copy(e->vals[i]); 
		}
	}
	return sval_err("Unbound symbol '%s'", k->sym); 
}

// update or bind new environmental symbols 
void env_put(env* e, sval* k, sval* v) {
	// if the sym exists, update it 
	for (int i = 0; i < e->count; i++) {
		if (strcmp(e->syms[i], k->sym) == 0) {
			sval_free(e->vals[i]); 
			e->vals[i] = sval_copy(v); 
			return;  
		}
	}

	// new binding 
	// allocate space for new entry 
	e->count++; 
	e->vals = realloc(e->vals, sizeof(sval*) * e->count); 
	e->syms = realloc(e->syms, sizeof(char*) * e->count); 
	// update contents 
	e->vals[e->count-1] = sval_copy(v); 
	e->syms[e->count-1] = malloc(strlen(k->sym)+1); 
	strcpy(e->syms[e->count-1], k->sym); 
	return; 

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
	if (strstr(t->tag, "qexpr"))  { x = sval_qexpr(); }

	// fill in the new S-Expr with any valid expressions 
	for (int i = 0; i < t->children_num; i++) {
		// weed through any meta-information 
		if (strcmp(t->children[i]->contents, "(") == 0) {continue;}
		if (strcmp(t->children[i]->contents, ")") == 0) {continue;}
		if (strcmp(t->children[i]->contents, "{") == 0) {continue;}
		if (strcmp(t->children[i]->contents, "}") == 0) {continue;}
		if (strcmp(t->children[i]->tag,  "regex") == 0) {continue;}
		x = sval_add(x, sval_read(t->children[i])); 
	}
	return x; 
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
		case SVAL_FUN: 	  printf("<function>"); 				break; 
		case SVAL_SEXPR:  sval_expr_print(v, '(', ')'); break; 
		case SVAL_QEXPR:  sval_expr_print(v, '{', '}'); break; 
	}
}

void sval_println(sval* v) { sval_print(v); putchar('\n'); }

// // // // // // // // // // 
// Evaluating Expressions  //
// // // // // // // // // // 


sval* sval_eval(env* e, sval* v); 
sval* sval_pop(sval* v, int i); 
sval* sval_take(sval* v, int i); 
sval* builtin_op(env* e, sval* v, char* op); 
sval* builtin(sval* a, char* func); 

sval* sval_eval_sexpr(env* e, sval* v) {
	// debug printf("sval_eval_sexpr() "); 
	// evaluate from the inside out 
	for (int i = 0; i < v->count; i++) {
		v->cell[i] = sval_eval(e, v->cell[i]); 
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
	if (f->type != SVAL_FUN) {
		sval_free(f); sval_free(v); 
		// throw syntax error 
		return sval_err("S-expression does not start with a valid symbol."); 
	}

	// pass expression and arguments to function
	sval* result = f->fun(e, v);  
	sval_free(f); 
	return result; 
}

sval* sval_eval(env* e, sval* v) {
	if (v->type == SVAL_SYM) {
		sval* x = env_get(e, v); 
		sval_free(v); 
		return x; 
	}
	// evaluate S-expressions 
	if (v->type == SVAL_SEXPR) { return sval_eval_sexpr(e, v); }
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
	sval* x = sval_pop(v, i); 
	sval_free(v); 
	return x; 
}


sval* builtin_op(env* e, sval* a, char* op) {
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

// Q-expr functionality // 

// general-purpose error handler 
#define ERRCHECK(args, cond, fmt, ...) \
	if (!(cond)) { \
		sval* err = sval_err(fmt, ##__VA_ARGS__); \
		sval_free(args); \
		return err; \
	}

sval* builtin_head(env* e, sval* a) {
	ERRCHECK(a, a->count == 1, 
		"Function 'head' passed too many arguments!"); 
	ERRCHECK(a, a->cell[0]->type == SVAL_QEXPR, 
		"Function 'head' passed the incorrect type!"
		" Got %s, expected %s.", 
		stype_name(a->cell[0]->type), stype_name(SVAL_QEXPR)); 
	ERRCHECK(a, a->cell[0]->count != 0, 
		"Function 'head' passed { }!"); 

	sval* v = sval_take(a, 0); 
	// returns only the head of the expression 
	while (v->count > 1) { sval_free(sval_pop(v, 1)); }
	return v; 
}

sval* builtin_tail(env* e, sval* a) {
	ERRCHECK(a, a->count == 1, 
		"Function 'tail' passed too many arguments!"); 
	ERRCHECK(a, a->cell[0]->type == SVAL_QEXPR, 
		"Function 'tail' passed the incorrect type!"); 
	ERRCHECK(a, a->cell[0]->count != 0, 
		"Function 'tail' passed { }!");
	// removes the head of the expression and returns the rest 
	sval* v = sval_take(a, 0); 
	sval_free(sval_pop(v, 0)); 
	return v; 
}

sval* builtin_list(env* e, sval* a) {
	// simple S->Q conversion 
	a->type = SVAL_QEXPR; 
	return a; 
}

sval* builtin_eval(env* e, sval* a) {
	ERRCHECK(a, a->count == 1, 
		"Function 'eval' passed too many arguments!"); 
	ERRCHECK(a, a->cell[0]->type == SVAL_QEXPR, 
		"Function 'eval' passed the incorrect type!"); 
	
	sval* x = sval_take(a, 0); 
	x->type = SVAL_SEXPR;
	return sval_eval(e, x); 
}

// join functionality needs a helper function, as it can take potentially many 
// arguments 
sval* sval_join(sval* x, sval* y) {
	// add each cell in 'y' to 'x'
	while (y->count) {
		x = sval_add(x, sval_pop(y, 0)); 
	}

	// delete the empty 'y', and return 'x'
	sval_free(y); 
	return x; 
}

sval* builtin_join(env* e, sval* a) {
	// ensure that all arguments are valid 
	for (int i = 0; i < a->count; i++) {
		ERRCHECK(a, a->cell[i]->type == SVAL_QEXPR, 
			"Function 'join' passed the incorrect type!"); 
	}
	
	sval* x = sval_pop(a, 0); 

	while (a->count) {
		// pop each value and append it to 'x'
		x = sval_join(x, sval_pop(a, 0)); 
	}

	// x replaces a as the joined expression 
	sval_free(a); 
	return x; 
}

// updates for environmental bindings 
sval* builtin_add(env* e, sval* a) {  return builtin_op(e, a, "+"); }
sval* builtin_sub(env* e, sval* a) {  return builtin_op(e, a, "-"); }
sval* builtin_mult(env* e, sval* a) { return builtin_op(e, a, "*"); }
sval* builtin_div(env* e, sval* a) {  return builtin_op(e, a, "/"); }
sval* builtin_pow(env* e, sval* a) {  return builtin_op(e, a, "^"); }
sval* builtin_max(env* e, sval* a) {  return builtin_op(e, a, "max"); }
sval* builtin_min(env* e, sval* a) {  return builtin_op(e, a, "min"); }
sval* builtin_mod(env* e, sval* a) {  return builtin_op(e, a, "%"); }

sval* builtin_def(env* e, sval* a) {
	ERRCHECK(a, a->cell[0]->type == SVAL_QEXPR, 
		"Function 'def' passed the incorrect type!"); 

	// first argument is a symbol list 
	sval* syms = a->cell[0]; 
	for (int i = 0; i < syms->count; i++) {
		ERRCHECK(a, syms->cell[i]->type == SVAL_SYM, 
			"Function 'def' cannot bind non-symbols!"); 
	}

	// check that the number of symbols and values is the same 
	ERRCHECK(a, syms->count == a->count -1, 
		"Function 'def' needs the same number of"
		" values and symbols!"); 

	// bind symbols to values in the target environment 
	for (int i = 0; i < syms->count; i++) {
		env_put(e, syms->cell[i], a->cell[i+1]); 
	}

	sval_free(a); 
	return sval_sexpr(); 
}

// register the builtins with the environment 
void env_add_builtin(env* e, char* name, sbuiltin func) {
	sval* k = sval_sym(name); 
	sval* v = sval_fun(func); 
	env_put(e, k, v); 
	sval_free(k); sval_free(v); 
}

void env_add_builtins(env* e) {
	env_add_builtin(e, "list", builtin_list);
	env_add_builtin(e, "head", builtin_head);
	env_add_builtin(e, "tail", builtin_tail);
	env_add_builtin(e, "eval", builtin_eval);
	env_add_builtin(e, "join", builtin_join);

	env_add_builtin(e, "+", builtin_add);
	env_add_builtin(e, "-", builtin_sub);
	env_add_builtin(e, "*", builtin_mult);
	env_add_builtin(e, "/", builtin_div);
	env_add_builtin(e, "max", builtin_max);
  env_add_builtin(e, "min", builtin_min);
	env_add_builtin(e, "^", builtin_pow);
	env_add_builtin(e, "%", builtin_mod);

	env_add_builtin(e, "def", builtin_def); 
}

// swtichboard for builtin functions 
/*  if (strcmp("list", func) == 0) { return builtin_list(a); }
  if (strcmp("head", func) == 0) { return builtin_head(a); }
  if (strcmp("tail", func) == 0) { return builtin_tail(a); }
  if (strcmp("join", func) == 0) { return builtin_join(a); }
  if (strcmp("eval", func) == 0) { return builtin_eval(a); }
  if (strstr("+-/*", func)) { return builtin_op(a, func); }
  sval_free(a);
  return sval_err("Unknown Function!");
} */ 


int main(int argc, char* argv[]) {

// Create Parsers 
mpc_parser_t* Number   = mpc_new("number"); 
mpc_parser_t* Symbol   = mpc_new("symbol");
mpc_parser_t* Sexpr 	 = mpc_new("sexpr");
mpc_parser_t* Qexpr 	 = mpc_new("qexpr");  
mpc_parser_t* Expr     = mpc_new("expr"); 
mpc_parser_t* Cue 		 = mpc_new("cue"); 

// Define Language 
mpca_lang(MPCA_LANG_DEFAULT, 
	"																			   							 \
		number   : /-?[0-9]+/ ; 							 							 \
		symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;				 \
	  sexpr    : '(' <expr>* ')' ; 												 \
	  qexpr 	 : '{' <expr>* '}' ; 												 \
		expr 		 : <number> | <symbol> | <sexpr> | <qexpr> ; \
		cue      : /^/ <expr>* /$/ ; 												 \
		", 
		Number, Symbol, Sexpr, Qexpr, Expr, Cue);   


	// print version and exit information 
	puts("Safiniea Version 0.0.1"); 
	puts("Ctrl-C to exit."); 

	// initialize top scope 
	env* e = env_new(); 
	env_add_builtins(e); 

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
			mpc_ast_print(r.output);
			sval* x = sval_eval(e, sval_read(r.output)); 
			sval_println(x);
			sval_free(x); 
			mpc_ast_delete(r.output); 
		} 
		else {
			// err 
			mpc_err_print(r.error); 
			mpc_err_delete(r.error); 
		}

		// free retrieved input
		free(input); 
	}
	env_del(e); 
	// undefine and clean up Parsers 
	mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Cue);
	return(EXIT_SUCCESS); 
}
