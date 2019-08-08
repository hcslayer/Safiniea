# Safiniea 
##### *A little lispy language.*
---

## Project Overview 

This project is based on orangeduck's fantastic [buildyourownlisp][r1]. My hope was to build a fun, flavorful little programming language in C. 

Safiniea is based on a small core of data types and operations: numbers (integers), strings, executibles (S-Expressions), non-executibles (Q-Expressions), errors, and booleans. This is nowhere near a production class language, but it has dignity and spunk, and it was a true joy to build. 

Safiniea also supports a number of features uncommon in other languages, such as the ability to manipulate code as data, partial function definition, variable argument functions, and more. This project is very much *in progress*, and the current source is littered with bugs and clutter. If it strikes your interest, stay tuned! Tour the program! Visit orangeduck's site! Use the fantastic [mpc library][r2] and start building things! Huzzah! 

---

## Quick Start Guide 

Safiniea can be run in two ways, both from the CLI. Simply running `prompt` or `./prompt` will fire up a console session, where you can type your Saf code into stdin and play around. 

Alternatively, you can do `./prompt file1 file2 ... ` to load files into the program. Each file will be parsed and evaluated in the sequence provided, and any and all outputs will be logged to the console. 

##### A note about syntax 

Safiniea uses reverse Polish syntax for all operations. It may be a bit unfamiliar, but I've found that it wears well. For the user unfamiliar with reverse Polish syntax, the general gist is that `<op1> <operator> <op2>` becomes `<operator> <op1> <op2> ...`

In other words, `(x + y)` is `(+ x y)`  
`8 * (7 + 3 (8 - 5))` would be `* 8 (+ 7 (* 3 (- 8 5)))`, etc. 

##### Arithmetic 

Safiniea currently has a single numeric data type: the humble integer. 
Safiniea supports addition, subtraction, multiplication, division, etc. 
`min` and `max` return the minimum or maximum value of an arbitrary list of arguments. 

*Modular arithmetic and exponentiation are currently in-progress*.  

##### Built-In Functions  

`function name` : description goes here 

##### Bindings  

##### Conditionals / Comparisons  

##### Strings 

##### Function Definition 

##### Partial Function Definition

##### Loading Files 

Put in a word here about format. Enclose each 'statement', so to speak, in an S-Expression. That's how we denote a line of executible code. Comments are denoted by '::' 

---

## Under the Hood 

--- 

## Development Bits 

#### Current Bugs 

*To be wrung out before moving to To Do*

-  ~~Something is being freed too many times, throws a malloc bp. I think that it has to do with the free_env() function.~~ 
-  Verify that function definition works. 
-  Verify that partial function definition works 
-  Verify that currying-uncurrying works 
-  Verify all arithmetic operations 
-  Verify Q-expr operations (head, tail, list) 
-  ~~Verify that bindings are preserved through a session~~ 
-  Verify that variable-argument functions work as expected 
-  ~~Test and evaluate load functions~~ 
-  Test standard library functions 


#### Looking Ahead 

-  Implement wrapper classes for file I/O
-  Implement AND / OR / NOT / XOR 
-  Implement a dedicated list type. 
-  Implement list manipulations (forEach, map, filter, remove) 
-  Clean and polish 

#### Back Burner 

-  Build a repl.it project so that the language can be demoed without cloning the repo 
-  Build out a string library 
-  Prettify everything. 

[r1]: http://www.buildyourownlisp.com
[r2]: https://github.com/orangeduck/mpc

