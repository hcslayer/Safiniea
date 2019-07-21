// basic REPL loop that will build the foundation for the parser 

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

typedef struct {
	int type; 
	long num; 
	int err; 
} sval; 

typedef enum {
	SVAL_NUM, 
	SVAL_ERR, 
} sValueType; 

typedef enum {
	SERR_ZERO_DIV, 
	SERR_BAD_OP, 
	SERR_BAD_NUM
} sErrType; 

// number type 
sval sval_num(long x) {
	sval v; 
	v.type = SVAL_NUM; 
	v.num = x; 
	return v; 
}

// error type 
sval sval_err(int x) {
	sval v; 
	v.type = SVAL_ERR; 
	v.err = x; 
	return v; 
}

// print sval 
void sval_print(sval v) {
	switch (v.type) {
		case SVAL_NUM: 
			printf("%li\n", v.num); 
			break;
		case SVAL_ERR:
			// error types 
			if (v.err == SERR_ZERO_DIV) {
				printf("ERROR: Division by zero.\n"); 
			} else if (v.err == SERR_BAD_OP) {
				printf("ERROR: Operator not recognized.\n"); 
			} else if (v.err == SERR_BAD_NUM) {
				printf("ERROR: Number not recognized.\n"); 
			} else {
				printf("An unknown error occured.\n"); 
			}
			break; 
	}
}



// switchboard to evaluate operations
sval eval_op(sval x, char* op, sval y) {

	// if either type is an error, return it 
	if (x.type == SVAL_ERR) { return x; }
	if (y.type == SVAL_ERR) { return y; }

	// else, perform operation 
	if (strcmp(op, "+") == 0) { return sval_num(x.num + y.num); }
	if (strcmp(op, "-") == 0) { return sval_num(x.num - y.num); }
	if (strcmp(op, "*") == 0) { return sval_num(x.num * y.num); }
	if (strcmp(op, "^") == 0) { return sval_num(pow(x.num, y.num)); }
	// following operations require nonzero operands 
	if (y.num == 0) { return sval_err(SERR_ZERO_DIV); }
	if (strcmp(op, "/") == 0) { return sval_num(x.num / y.num); }
	if (strcmp(op, "%") == 0) { return sval_num(x.num % y.num); }
	
	return sval_err(SERR_BAD_OP); 
}

sval eval(mpc_ast_t* t) {

	if (strstr(t->tag, "number")) {
		// determine if number is valid 
		errno = 0; 
		long x = strtol(t->contents, NULL, 10); 
		return errno != ERANGE ? sval_num(x) : sval_err(SERR_BAD_NUM); 
	}
	// parse operator, the second child 
	char* op = t->children[1]->contents; 

	// store the third child in `x`
	sval x = eval(t->children[2]); 

	// iterate the remaining children, combining 
	// the result from each expression 
	int i = 3; 
	while (strstr(t->children[i]->tag, "expr")) {
		x = eval_op(x, op, eval(t->children[i])); 
		i++; 
	}
	return x; 
}

int main(int argc, char* argv[]) {

// Create Parsers 
mpc_parser_t* Number   = mpc_new("number"); 
mpc_parser_t* Operator = mpc_new("operator");
mpc_parser_t* Expr 		 = mpc_new("expr"); 
mpc_parser_t* Cue 		 = mpc_new("cue"); 

// Define Language 
mpca_lang(MPCA_LANG_DEFAULT, 
	"																			   							\
		number   : /-?[0-9]+/ ; 							 							\
		operator : '+' | '-' | '/' | '*' | '%' | '^' |			\
								/min/ | /max/ ; 												\
		expr 		 : <number> | '(' <operator> <expr>+ ')' ;	\
		cue      : /^/ <operator> <expr>+ /$/ ; 						\
		", 
		Number, Operator, Expr, Cue);   


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
			mpc_ast_print(r.output);
			sval result = eval(r.output); 
			sval_print(result); 

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
	mpc_cleanup(4, Number, Operator, Expr, Cue);
	return(EXIT_SUCCESS); 
}