/* SAFINIEA 
 * A lisp invariant. 
 * 
 * Henry Slayer 
 * Last Updated: July 31 2019 
 */ 

/* Internal Notes: 
 * 
 * TODO: 
 * 			[ ] Refactor and prettify. 
 *			[ ] Convert static arrays to linked lists. 
 *			[ ] Review how everything actually works. 
 *			[ ] Hot comment boxes for everything 
 * 			[ ] Merge this branch with master. 
 */ 

#include<stdio.h> 
#include<stdlib.h> 
#include<stdbool.h>
#include<math.h> 
#include"mpc.h" 		/* Deep thanks to orangeduck for inspiration and guidance! */ 
#include"List.h"

/* Windows-specific configuration */ 
#ifdef 		_WIN32 
#include  <string.h>
#define 	MAXLINE	2048

static char buffer[2048]; 

char* readline(char* prompt) {
	fputs(prompt, stdout); 
	fgets(buffer, MAXLINE, stdin); 
	char* in = malloc(strlen(buffer)+1); 
	strcpy(in, buffer); 
	cpy[strlen(in)-1] = '\0'; 	/* chop newline */ 
	return in; 
}

void add_history(char* reserve) {/* space filler */} 

#else 
#include <editline/readline.h>
#endif 


/*********************/ 
/** Structure types **/
/*********************/ 

struct sval; 
struct env; 
typedef struct sval sval; 
typedef struct env env; 
typedef sval*(*sbuiltin)(env*, sval*); 

/* All parsed data is of type sval */ 

/* Sub-types */ 
typedef enum {
	SVAL_NUM, SVAL_ERR, SVAL_SYM, SVAL_SEXPR, 
	SVAL_QEXPR, SVAL_FUN
} value_type; 

typedef enum {
	ZERO_DIV_ERR, BAD_OP_ERR, BAD_NUM_ERR
} err_type; 

/* Error handling macros */ 
#define ERRCHECK(args, cond, fmt, ...) \
	if (!(cond)) { \
		sval* err = error(fmt, ##__VA_ARGS__); \
		free_sval(args); \
		return err; \
	}

/* Helper function for type lookup */ 
char* find_type(value_type t) {
	switch(t) {
		case SVAL_FUN: return "Function"; 
		case SVAL_NUM: return "Number"; 
		case SVAL_SYM: return "Symbol"; 
		case SVAL_ERR: return "Error"; 
		case SVAL_SEXPR: return "S-Expression"; 
		case SVAL_QEXPR: return "Q-Expression"; 
		default: return "???"; 
	}
}

typedef struct sval {
	value_type type; 

	/* Basics */ 
	long num; 
	char* err; 
	char* sym; 

	/* Functions */ 
	sbuiltin builtin;
	env* env; 
	sval* formals; 
	sval* body; 

	/* Nested elements */
	List children; 
	int count;
} sval; 

/* Sub-type constructors */

/* Number */  
sval* number(long x) {
	sval* v = malloc(sizeof(sval));  
	v->type = SVAL_NUM;
	v->num = x; 
	return v; 
}

/* Error */ 
sval* error(char* fmt, ...) {
	sval* v = malloc(sizeof(sval)); 
	v->type = SVAL_ERR; 
	
	/* Init and configure v.a. list */ 
	va_list va; 
	va_start(va, fmt);  
	v->err = malloc(512); 
	vsnprintf(v->err, 511, fmt, va); 
	
	/* Err messages max out at 511 bytes */ 
	v->err = realloc(v->err, strlen(v->err)+1);
	va_end(va); 

	return v; 
}

/* Symbol */ 
sval* symbol(char* s) {
	sval* v = malloc(sizeof(sval)); 
	v->type = SVAL_SYM; 
	v->sym  = malloc(strlen(s)+1); 
	strcpy(v->sym, s); 
	return v; 
}

/* S-Expression */ 
sval* sexpr(void) {
	sval* v = malloc(sizeof(sval)); 
	v->type = SVAL_SEXPR; 
	v->count = 0; 
	v->children = newList(); 
	return v; 
}

/* Q-Expression */ 
sval* qexpr(void) {
	sval* v = malloc(sizeof(sval)); 
	v->type = SVAL_QEXPR;
	v->count = 0; 
	v->children = newList();
	return v; 
}

/* Function */ 
sval* fun(sbuiltin func) {
	sval* v = malloc(sizeof(sval)); 
	v->type = SVAL_FUN; 
	v->builtin = func; 
	return v; 
}

/* Environment structure */ 
struct env {
	int count; 
	env* parent; 
	char** syms; 
	sval** vals; 
}; 

env* new_env(void) {
	env* e = malloc(sizeof(env)); 
	e->parent = NULL; 
	e->count = 0; 
	e->syms = NULL; 
	e->vals = NULL; 
	return e; 
}

/* Destructors */ 
void free_env(env* e); 
void free_sval(sval* v); 

void free_sval(sval* v) {

	switch(v->type) {

		case SVAL_NUM: break;  

		case SVAL_ERR: free(v->err); break; 
		case SVAL_SYM: free(v->sym); break; 

		case SVAL_SEXPR: 
		case SVAL_QEXPR:
			freeList(&(v->children));  
			break; 

		case SVAL_FUN: 
			if (!v->builtin) {
				free_env(v->env); 
				free_sval(v->formals); 
				free_sval(v->body); 
			}
			break; 
	}
	free(v); 
}

void free_env(env* e) {
	for (int i = 0; i < e->count; i++) {
		free(e->syms[i]); 
		free_sval(e->vals[i]); 
	}
	free(e->syms); 
	free(e->vals); 
	free(e);
}

/********************/ 
/** Copy functions **/
/********************/

sval* copy_sval(sval* v); 
env* copy_env(env* e); 
List copy_list(List L); 


sval* copy_sval(sval* v) {
	/* Initialize duplicate */ 
	sval* x = malloc(sizeof(sval)); 
	x->type = v->type; 
	
	switch(v->type) {
		case SVAL_NUM: x->num = v->num; break; 
		
		/* Copy internal data */ 
		case SVAL_ERR: 
			x->err = malloc(strlen(v->err)+1); 
			strcpy(x->err, v->err); 
			break;
		case SVAL_SYM: 
			x->sym = malloc(strlen(v->sym)+1); 
			strcpy(x->sym, v->sym); 
			break; 
		
		/* Copy internal lists/linkage */ 
		case SVAL_SEXPR:
		case SVAL_QEXPR:
			x->count = v->count; 
			x->children = copy_list(v->children); 
			break;
		
		case SVAL_FUN: 
			if (v->builtin) {
				x->builtin = v->builtin; 
			} else {
				x->builtin = NULL; 
				x->env = copy_env(v->env); 
				x->formals = copy_sval(v->formals); 
				x->body = copy_sval(v->body); 
			}
			break; 
	}
	return x; 
}

env* copy_env(env* e) {
	env* n = malloc(sizeof(env)); 
	n->parent = e->parent; 
	n->count = e->count; 
	n->syms = malloc(sizeof(char*) * n->count); 
	n->vals = malloc(sizeof(char*) * n->count); 
	for (int i = 0; i < e->count; i++) {
		n->syms[i] = malloc(strlen(e->syms[i])+1); 
		strcpy(n->syms[i], e->syms[i]); 
		n->vals[i] = copy_sval(e->vals[i]); 
	} 
	return n; 
}

List copy_list(List L) {
	if (!L) { exit(EXIT_FAILURE); } /* FIXME: Do proper list error handling */ 
	List C = newList(); 						/* FIXME: Lint List camel-case to GNU std */ 
	sval* v; 
	if (length(L) > 0) {
		moveFront(L); 
		while (idx(L) >= 0) {
			v = (sval*)get(L); 
			append(C, v); 
			moveNext(L); 
		}
	}
	return C; 
}

/**************************/ 
/** Environment handlers **/ 
/**************************/ 

/* Retrieve environmental bindings */ 
sval* get_env(env* e, sval* k) {
	for (int i = 0; i < e->count; i++) {
		if (strcmp(e->syms[i], k->sym) == 0) {
			return copy_sval(e->vals[i]); 
		}
	}
	/* Symbol not found, search parent envs */ 
	if (e->parent) {
		return get_env(e->parent, k); 
	}
	return error("Unbound symbol '%s'", k->sym); 
}

/* Set environmental bindings */ 
void set_env(env* e, sval* k, sval* v) {
	
	/* Update an existing binding */ 
	for (int i = 0; i < e->count; i++) {
		if (strcmp(e->syms[i], k->sym) == 0) {
			free_sval(e->vals[i]); 
			e->vals[i] = copy_sval(v); 
			return;  
		}
	}

	/* Create a new binding */ 
	e->count++; 
	e->vals = realloc(e->vals, sizeof(sval*) * e->count); 
	e->syms = realloc(e->syms, sizeof(char*) * e->count); 
	// update contents 
	e->vals[e->count-1] = copy_sval(v); 
	e->syms[e->count-1] = malloc(strlen(k->sym)+1); 
	strcpy(e->syms[e->count-1], k->sym); 
	return; 
}

/* Wrapper to update top scope */ 
void def_env(env* e, sval* k, sval* v) {
	while (e->parent) { e = e->parent; }; 
	set_env(e, k, v); 
}

/* Function builder */ 
sval* make_func(sval* formals, sval* body) {
	sval* v = malloc(sizeof(sval)); 
	v->type = SVAL_FUN; 

	/* Signifies user-created */ 
	v->builtin = NULL; 

	/* Internal scope */ 
	v->env = new_env(); 
	v->formals = formals; 
	v->body = body; 
	return v; 
}

/***********************/ 
/** Reading functions **/
/***********************/  

/* S-Expression helper: nests one sexpr inside the other */ 
sval* sexpr_compose(sval* v, sval* x) {
	v->count++; 
	append(v->children, x); 
	return v; 
}

/* Number reader with error handling */ 
sval* read_num(mpc_ast_t* t) {
	errno = 0; 
	long x = strtol(t->contents, NULL, 10); 
	return errno != ERANGE ? 
		number(x) : error("invalid number"); 
}

/* MPC reader, parses AST. 
 * Do not confuse t->children with children of values */ 
sval* sval_read(mpc_ast_t* t) {
	
	/* Convert symbols to the proper type */ 
	if (strstr(t->tag, "number")) { return read_num(t); }
	if (strstr(t->tag, "symbol")) { return symbol(t->contents); }
	
	/* S-expressions and Q-expressions simply hold more data */ 
	sval* x = NULL; 
	if (strcmp(t->tag, ">") == 0) { x = sexpr(); }
	if (strstr(t->tag, "sexpr")) 	{ x = sexpr(); }
	if (strstr(t->tag, "qexpr"))  { x = qexpr(); }

	/* Recurse to fill in that data */ 
	for (int i = 0; i < t->children_num; i++) {
		
		/* Consume and discard any unnecessary signage */ 
		if (strcmp(t->children[i]->contents, "(") == 0) {continue;}
		if (strcmp(t->children[i]->contents, ")") == 0) {continue;}
		if (strcmp(t->children[i]->contents, "{") == 0) {continue;}
		if (strcmp(t->children[i]->contents, "}") == 0) {continue;}
		if (strcmp(t->children[i]->tag,  "regex") == 0) {continue;}
		printf("calling sexpr_compose()\n"); 
		x = sexpr_compose(x, sval_read(t->children[i])); 
	}
	return x; 
}

/************************/ 
/** Printing functions **/
/************************/ 

void print_expr(sval* v, char open, char close); 
void print_value(sval* v);
void print_valueln(sval* v); 

void print_expr(sval* v, char open, char close) {
	putchar(open); 
	if (length(v->children) > 0) {
		moveFront(v->children); 
		while (idx(v->children) >= 0) {
			print_value(get(v->children));
			if (idx(v->children) < v->count - 1) {
				putchar(' '); 
			} 
			moveNext(v->children); 
		}
	}
	putchar(close); 
}

void print_value(sval* v) {
	switch (v->type) {
		case SVAL_NUM: 		printf("%li", v->num); 			  break; 
		case SVAL_ERR: 		printf("Error: %s", v->err);  break; 
		case SVAL_SYM: 		printf("%s", v->sym); 			  break; 
		case SVAL_SEXPR:  print_expr(v, '(', ')'); break; 
		case SVAL_QEXPR:  print_expr(v, '{', '}'); break; 
		case SVAL_FUN: 		
			if (v->builtin) { printf("<builtin>"); } 
			else {
				printf("(\\ "); print_value(v->formals); 
				putchar(' '); 	print_value(v->body); putchar(')');  
			}
			break; 
	}
}

void print_valueln(sval* v) { print_value(v); putchar('\n'); }

/**************************/ 
/** Evaluation functions **/
/**************************/ 

sval* evaluate(env* e, sval* v); 
sval* evaluate_sexpr(env* e, sval* v); 
sval* pop(sval* v, int i); 
sval* take(sval* v, int i); 
sval* builtin_op(env* e, sval* v, char* op); 
sval* builtin(sval* a, char* func); 
sval* call(env* e, sval* f, sval* a);

/* Directs evaluation; main entry point */ 
sval* evaluate(env* e, sval* v) {
	if (v->type == SVAL_SYM) {
		printf("symbol detected\n"); 
		/* Retrieve binding value */ 
		sval* x = get_env(e, v); 
		free_sval(v); 
		return x; 
	}

	/* S-expressions are evaluated */ 
	if (v->type == SVAL_SEXPR) {
		return evaluate_sexpr(e, v); 
	}

	/* Any other type is returned */ 
	return v; 
}

/* Swap: a list helper for evaluate_sexpr */ 
void swap(List L, sval* x) {
	int target_index = idx(L);
	printf("Target index: %d\n", target_index);  
	insertBefore(L, x); /* insert new element */ 
	delete(L); /* delete old element at target idx */ 
	moveFront(L); /* restore cursor position */ 
	for (int i = 0; i < target_index; i++) {
		moveNext(L); 
	}
}

/* S-expression evaluator; works from inside -> out */  
sval* evaluate_sexpr(env* e, sval* v) {
	printf("eval_sexpr(): v->count: %d\n", v->count); 
	List children = v->children; 
	if (v->count > 0) { moveFront(children); }
	for (int i = 0; i < v->count; i++) {
		sval* child_eval = evaluate(e, get(children)); 
		swap(children, child_eval); 
		moveNext(children); 
	}

	/* Check for error propogation */
	if (v->count > 0) { moveFront(children); } 
	for (int i = 0; i < v->count; i++) {
		if (((sval*)get(children))->type == SVAL_ERR) {
			return take(v, 0); 
		}
	}

	/* Case of 1 and 0 */ 
	if (v->count == 0) { return v; }
	if (v->count == 1) { return evaluate(e, take(v, 0)); }
	
	/* Evaluation:  ensure that the first element is a symbol */ 
	sval* f = pop(v, 0); 
	if (f->type != SVAL_FUN) {
		sval* err = error(
			"S-Expression starts with incorrect type. "
			"Got %s, expected %s.", 
			find_type(f->type), find_type(SVAL_FUN)); 
		return err; 
	}

	sval* result = call(e, f, v); 
	free_sval(f); 
	return result; 
}

/* Pop : removes an element from the list, leaving the remainder in tact */ 
sval* pop(sval* v, int i) {
	List V = v->children; 
	moveFront(V); 
	for (int k = 0; k < i; k++) {
		moveNext(V); 
	}
	sval* x = get(V); 
	delete(V); 
	v->count--; 
	return x; 
	/* 
	sval* x; 
	if (i == 0 && length(v->children) > 0) {
		x = (sval*)front(v->children);
		deleteFront(v->children); 
		v->count--; 
		return x; 
	}
	
	if (length(v->children) > 0) {
		moveFront(v->children); 
		while (idx(v->children) < i && idx(v->children) != -1) {
			moveNext(v->children); 
		}
		if (idx(v->children) != -1) { 
			x = (sval*)get(v->children); 
			delete(v->children);
			v->count--;  
			return x; 
		} else {
			return error("Henry, your linked list is broken. Fix this."); 
		}
	}
	return error("Unhandled error in sval* pop()"); 
	*/
}

/* Take : pops the ith element, and frees the rest */ 
sval* take(sval* v, int i) {
	sval* x = pop(v, i); 
	free_sval(v); 
	return x; 
}


/* Helpers for builtin_op */ 
long findMin(long x, long y) { return (x > y) ? y : x; }
long findMax(long x, long y) { return (x > y) ? x : y; }

/* Switchboard for evaluating basic built-ins (+-/*) */
sval* builtin_op(env* e, sval* a, char* op) {
	/* Exit */ 
	if (strcmp(op, "Exit") == 0) { 
		printf("Goodbye!\n"); 
		free_sval(a); 
		/* free_env(e); not sure if necessary */ 
		exit(EXIT_SUCCESS); 
	}

	/* Verify that input are numbers */ 
	List A = a->children; 
	if (length(A) > 0) {
		moveFront(A);
		while (idx(A) >= 0) {
			if (((sval*)get(A))->type != SVAL_NUM) {
				free_sval(a); 
				return error("%s Error: Cannot operate on non-number arguments", op); 
			}
			moveNext(A); 
		}
	}

	/* Pop the first element */ 
	sval* x = pop(a, 0); 

	/* If no arguments, and op == '-', negate */ 
	if ((strcmp(op, "-") == 0 && a->count == 0)) {
		x->num = -1 * x->num; 
	}

	/* Evaluate arguments pairwise, accumulating the result */ 
	while (a->count > 0) {
		/* Next argument */ 
		sval* y = pop(a, 0); 

		/* Evaluate */  
		if (strcmp(op, "+") == 0) { x->num += y->num; }
		if (strcmp(op, "-") == 0) { x->num -= y->num; }
		if (strcmp(op, "*") == 0) { x->num *= y->num; }
		if (strcmp(op, "^") == 0) { x->num = pow(x->num, y->num); }
		if (strcmp(op, "max") == 0) { x->num = findMax(x->num, y->num); }
		if (strcmp(op, "min") == 0) { x->num = findMin(x->num, y->num); }
		
		/* Ops that require nonzero operands */ 
		if (y->num == 0) { 
			free_sval(x); free_sval(y); 
			x = error("Division by zero!"); break; 
		}
		if (strcmp(op, "/") == 0) { x->num = x->num / y->num; }
		if (strcmp(op, "%") == 0) { x->num = x->num % y->num; }

		free_sval(y); 
	}

	free_sval(a); 
	return x;
}

/*******************/ 
/** Q-Expressions **/ 
/*******************/ 

/* Head: resolves a Q-Expr into the first argument */ 
sval* builtin_head(env* e, sval* a) {
	/* Error handling */ 
	List A = a->children; 
	ERRCHECK(a, a->count == 1, 
		"Function 'head' passed too many arguments!"); 
	ERRCHECK(a, ((sval*)front(A))->type == SVAL_QEXPR, 
		"Function 'head' passed the incorrect type!"
		" Got %s, expected %s.", 
		find_type(((sval*)front(A))->type), find_type(SVAL_QEXPR)); 
	ERRCHECK(a, ((sval*)front(A))->count != 0, 
		"Function 'head' passed { }!"); 

	sval* v = take(a, 0); 
	/* Resolve only the first data element */  
	while (v->count > 1) { free_sval(pop(v, 1)); }
	return v; 
}

/* Tail: resolves by removing the head, returning what is left */ 
sval* builtin_tail(env* e, sval* a) {
	/* Error handling */ 
	List A = a->children; 
	ERRCHECK(a, a->count == 1, 
		"Function 'tail' passed too many arguments!"); 
	ERRCHECK(a, ((sval*)front(A))->type == SVAL_QEXPR, 
		"Function 'tail' passed the incorrect type!"
		" Got %s, expected %s.", 
		find_type(((sval*)front(A))->type), find_type(SVAL_QEXPR)); 
	ERRCHECK(a, ((sval*)front(A))->count != 0, 
		"Function 'tail' passed { }!"); 

	sval* v = take(a, 0); 
	/* Resolve the remainder */  
	free_sval(pop(v, 0)); 
	return v; 
}

/* List: converts a Q-Expr to an S-Expr */ 
sval* builtin_list(env* e, sval* a) {
	a->type = SVAL_QEXPR; 
	return a; 
}

/* Join: composes multiple Q-expressions */ 
sval* join_helper(sval* x, sval* y) {
	/* Add each cell in 'y' to 'x' */ 
	while (y->count) {
		x = sexpr_compose(x, pop(y, 0)); 
	}

	/* Discard 'y' */ 
	free_sval(y); 
	return x; 
}

sval* builtin_join(env* e, sval* a) {
	List A = a->children; 
	/* Validate args */ 
	if (length(A) > 0) {
		moveFront(A); 
		while (idx(A) >= 0) {
			ERRCHECK(a, ((sval*)get(A))->type == SVAL_QEXPR, 
				"Function 'join' passed the incorrect type."); 
		}
	}

	sval* x = pop(a, 0); 
	while (a->count) {
		x = join_helper(x, pop(a, 0)); 
	}

	/* x reaplces a as the joined expression */ 
	free_sval(a); 
	return x; 
}

/* Eval: converts Q-Expr to S-Expr and evaluates */ 
sval* builtin_eval(env* e, sval* a) {
	ERRCHECK(a, a->count == 1, 
		"Function 'eval' passed too many arguments!"); 
	ERRCHECK(a, ((sval*)front(a->children))->type == SVAL_QEXPR, 
		"Function 'eval' passed the incorrect type!"); 

	sval* x = take(a, 0); 
	x->type = SVAL_SEXPR; 
	return evaluate(e, x); 
}

/*********************************/ 
/** Functions && Environment II **/
/*********************************/

/* Register builtin functions with the ENV */ 
sval* builtin_add(env* e, sval* a) {  return builtin_op(e, a, "+"); }
sval* builtin_sub(env* e, sval* a) {  return builtin_op(e, a, "-"); }
sval* builtin_mult(env* e, sval* a) { return builtin_op(e, a, "*"); }
sval* builtin_div(env* e, sval* a) {  return builtin_op(e, a, "/"); }
sval* builtin_pow(env* e, sval* a) {  return builtin_op(e, a, "^"); }
sval* builtin_max(env* e, sval* a) {  return builtin_op(e, a, "max"); }
sval* builtin_min(env* e, sval* a) {  return builtin_op(e, a, "min"); }
sval* builtin_mod(env* e, sval* a) {  return builtin_op(e, a, "%"); }
sval* builtin_exit(env* e, sval* a) { return builtin_op(e, a, "exit"); } 

void env_add_builtin(env* e, char* name, sbuiltin func) {
	sval* k = symbol(name); 
	sval* v = fun(func); 
	set_env(e, k, v); 
	free_sval(k); free_sval(v); 
}


/* Setting variables */ 
sval* set_var(env* e, sval* a, char* func) {
	/* TODO: Implement type-checking here */ 

	/* Parse binding name */ 
	sval* symbols = (sval*)front(a->children); 
	List S = symbols->children; 
	List A = a->children; 
	/* Verify name */ 
	if (length(S) > 0) {
		moveFront(S); 
		while (idx(S) >= 0) {
			ERRCHECK(a, ((sval*)get(S))->type == SVAL_SYM, 
			"Function '%s' cannot define a non-symbol. "
			"Got %s, expected %s.", func, 
			find_type(((sval*)get(S))->type), find_type(SVAL_SYM)); 
			moveNext(S); 
		}
	} else { printf("Unexpected error in set_var(), list empty?"); exit(EXIT_FAILURE); } 

	ERRCHECK(a, (symbols->count == a->count - 1), 
		"Function '%s' was passed too many arguments for symbols. "
		"Got %i, expecting %i.", func, symbols->count, a->count -1); 

	/* "def" sets global bindings, "=" is a local binding */ 
	moveFront(S); moveFront(A); moveFront(A); /* Advance A one past S */ 
	for (int i = 0; i < symbols->count; i++) {
		if (strcmp(func, "def") == 0) {
			def_env(e, (sval*)get(S), (sval*)get(A)); 
		}
		if (strcmp(func, "=") == 0) {
			set_env(e, (sval*)get(S), (sval*)get(A)); 
		}
		moveNext(S); moveNext(A); 
	}

	/* Free a and return an empty S-Expr */ 
	free_sval(a); 
	return sexpr(); 
} 

/* Defines user-entered functions */ 
sval* builtin_lambda(env* e, sval* a) {
	List A = a->children; 
	if (length(A) > 0) {
		moveFront(A); 
		while (idx(A) >= 0) {
			ERRCHECK(a, ((sval*)get(A))->type == SVAL_SYM, 
			"Cannot define a non-symbol. "
			"Got %s, expected %s.", 
			find_type(((sval*)get(A))->type), find_type(SVAL_SYM)); 
			moveNext(A); 
		}
	} else { printf("Unexpected error in builtin_lambda(), list empty?"); exit(EXIT_FAILURE); }

	/* Pop the first two arguments, pass them as formal args and fcn body */ 
	sval* formals = pop(a, 0); 
	sval* body 		= pop(a, 0); 
	free_sval(a); 

	return make_func(formals, body); 
}

/* Call: Evaluates a function, either user-defined or global */ 
sval* call(env* e, sval* f, sval* a) {
	if (f->builtin) { return f->builtin(e, a); }

	/* record argument counts */ 
	int given = a->count; 
	int total = f->formals->count; 

	while (a->count) {
		/* No formal arguments remain */ 
		if (f->formals->count == 0) {
			free_sval(a); return error(
				"Function construction failed, too many arguments! "
				"Got %i, only space for %i.", given, total); 
		}
		
		/* Pop the first symbol and evaluate */ 
		sval* symbol = pop(f->formals, 0); 

		/* Variable argument definition */ 
		if (strcmp(symbol->sym, "...") == 0) { 
			if (f->formals->count != 1) {
				free_sval(a); 
				return error("Variable Argument definition invalid. "
					"'...' is to be followed by a single symbol. "); 
			}
			/* next formal is set to remaining arguments */ 
			sval* nsym = pop(f->formals, 0); 
			set_env(f->env, nsym, builtin_list(e, a)); 
			free_sval(symbol); free_sval(nsym); 
		}

		/* Standard definition */ 
		sval* val = pop(a, 0); 
		set_env(f->env, symbol, val); 
		free_sval(symbol); free_sval(val);
	}

	/* All arguments have been bound */ 
	free_sval(a); 

	/* If VA symbols remain, bind to an empty list */ 
	if (f->formals->count > 0 
		&& strcmp(((sval*)front(f->formals->children))->sym, "...") == 0) {

		/* verify that '...' doesn't stand alone */ 
		if (f->formals->count != 2) {
			return error("Function definition invalid. "
				"'...' must be followed by a single symbol."); 
		}

		/* pop and delete '...' */ 
		free_sval(pop(f->formals, 0)); 

		/* pop the next symbol, create an empty list */ 
		sval* symbol = pop(f->formals, 0); 
		sval* list   = qexpr();

		/* Set env bindings */ 
		set_env(f->env, symbol, list); 
		free_sval(symbol); free_sval(list);  
	}

	/* If the number of arguments matched expectations, evaluate */ 
	if (f->formals->count == 0) {
		f->env->parent = e; 
		return builtin_eval(
			f->env, sexpr_compose(sexpr(), copy_sval(f->body))); 
	} 
	else { /* Partial function definition */ 
		return copy_sval(f); 
	}
}

/* Put and define register values to the local or global namespace */ 
sval* builtin_def(env* e, sval* a) {
	return set_var(e, a, "def");
}

sval* builtin_put(env* e, sval* a) {
	return set_var(e, a, "="); 
}

/* Configures the environment with standard functionality */ 
void env_configure(env* e) {
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
	env_add_builtin(e, "exit", builtin_exit); 

	env_add_builtin(e, "def", builtin_def); 
	env_add_builtin(e, "\\", builtin_lambda); 
	env_add_builtin(e, "=", builtin_put); 
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int main(int argc, char* argv[]) {

	/* Parsers */ 
	mpc_parser_t* Number 	= mpc_new("number"); 
	mpc_parser_t* Symbol  = mpc_new("symbol"); 
	mpc_parser_t* Sexpr		= mpc_new("sexpr"); 
	mpc_parser_t* Qexpr 	= mpc_new("qexpr"); 
	mpc_parser_t* Expr 		= mpc_new("expr"); 
	mpc_parser_t* Cue			= mpc_new("cue"); 

	/* Grammars */ 
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

	/* Version, exit information */ 
	puts("Safiniea Version 0.2.0"); 
	puts("'Exit 1' for a hot exit."); 

	/* Initialize top scope */ 
	env* e = new_env(); 
	env_configure(e); 

	/* REPL */ 
	while (true) {

		char* input = readline("\n∞ > ");
		add_history(input);

		/* Parse user input */ 
		mpc_result_t r; 
		if (mpc_parse("<stdin>", input, Cue, &r)) {
			/* Diagnostic: print AST */ 
			mpc_ast_print(r.output);
			sval* x = evaluate(e, sval_read(r.output)); 
			print_valueln(x);
			free_sval(x); 
			mpc_ast_delete(r.output); 
		} 
		/* Parsing error */ 
		else {
			mpc_err_print(r.error); 
			mpc_err_delete(r.error); 
		}

		free(input); 
	}

	/* Clean up */ 
	free_env(e); 
	mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Cue); 

	exit(EXIT_SUCCESS); 
}
