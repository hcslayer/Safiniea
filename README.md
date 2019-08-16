# Safiniea 
##### *A little lispy language.*
---

## Project Overview 

This project is based on orangeduck's fantastic [buildyourownlisp][r1]. My hope was to build a fun, flavorful little programming language in C. 

Safiniea is based on a small core of data types and operations: numbers (integers), strings, executibles (S-Expressions), non-executibles (Q-Expressions), errors, and booleans. This is nowhere near a production class language, but it has dignity and spunk, and it was a true joy to build. 

Safiniea also supports a number of features uncommon in other languages, such as the ability to manipulate code as data, partial function definition, variable argument functions, and more. This project is very much *in progress*, and the current source is *proudly* littered with bugs and clutter. If it strikes your interest, stay tuned! Tour the program! Visit orangeduck's site! Use the fantastic [mpc library][r2] and start building your own lispy things! Huzzah! 

---

## Quick Start Guide 

Safiniea can be run in two ways, both from the CLI. Simply running `prompt` or `./prompt` will fire up a console session, where you can type your Saf code into stdin and play around. 

Alternatively, you can do `./prompt file1 file2 ... ` to load files into the program. Each file will be parsed and evaluated in the sequence provided, and any and all outputs will be logged to the console. Be sure to wrap all statements in `( ... )`, so that they will be properly parsed and evaluated. Otherwise, things will yell at you. 

In Safiniea, there are two main types of expressions: S `( ... )` and Q `{ ... } `. S-Expressions represent, more or less loosely, evaluable statements, while Q-Expressions are not evaluated. A single operator followed by an appropriately-formatted list of operands does not require padding in parentheses, however, more complex statements and compound expressions will likely use a combination of both. 

All strings must be enclosed in double inverted commas. 

Comments begin with `::` and are terminated by the first newline character encountered.  

##### A quick note about syntax 

Safiniea uses reverse Polish syntax for all operations. It may be a bit unfamiliar, but I've found that it wears well. For the user unfamiliar with reverse Polish syntax, the general gist is that `<op1> <operator> <op2>` becomes `<operator> <op1> <op2> ...`

In other words, `(x + y)` is `(+ x y)`  
`8 * (7 + 3 (8 - 5))` would be `* 8 (+ 7 (* 3 (- 8 5)))`, etc. 

The same sort of logic applies to operations beyond arithmetic, too. 

---

#### Example Code 

Here's an example session at the console, which demonstrates some of the basic functionality of the language: 

```
print "Hello World!" 
"Hello World"

+ 5 4 3 2 1 
15 

- 5 4 3 2 1 
-5 

* 5 4 3 2 1 
120 

min 5 4 3 2 1
1

max 5 4 3 2 1 
5 

def {x} 5								
:: symbol 'x' is now bound to 5 

def {name} "Safiniea"		
:: symbol 'name' is now bound to "Safiniea"

(\ {x y} {+ x y})						
:: (\ {args} {body} ) defines a function. 

def {plus-plus} (\ {x y} {+ x y}) 
:: binds a function definition to the symbol 'plus-plus'. 

plus-plus 6 5 
11 

plus-plus x 7 
12 

plus-plus x x x
Error: Function expected 2 arguments, got 3! 

list 8 7 6 3 
{8 7 6 3} 
:: 'list' returns a Q-expr with the given arguments 

head {8 7 6 4} 
8 
:: head < Q-Expr > returns the first element of the Q-Expression 

tail {1 2 3} 
{2 3} 
:: 'tail' pops the head off of a Q-Expression and returns the rest. 

join {4 5} {"fooey!"} 
{4 5 "fooey!"} 

!if {< x 6} {def {x} 6} {print "x is greater than 6"}
:: '!if' is a ternary-esque operator 
:: the definition of x is updated to 6 

== 7 8 
false 

!= 7 8 
true 

=== {6 7 8} {6 7 8} 
true 

=== {"six" "seven" "eight"} {6 7 8} 
false 
:: use '==' for numeric equality, and '===' for general equality 


```

---

##### Arithmetic 

Safiniea currently has a single numeric data type: the humble integer. 
Safiniea supports addition, subtraction, multiplication, division, etc. 
`min` and `max` return the minimum or maximum value of an arbitrary list of arguments. 

Make note that if you pass more than two operands to a binary operator, the result will be evaluated pairwise. That is,  
`* a b c ... ` returns `(a * b) * c) * ... `

Most arithmetic in Safiniea is relatively intuitive. Zero division throws errors, and you can't (theoretically) do anything that a calculator wouldn't let you do.   

---

##### Built-In Functions  

`function name` : description goes here 

`+ - * / % ^ ` : Arithmetic as usual. 

`min` and `max` : return minimum or maximum value (integers). 

`== != <= >= > < ` : Integer comparison as usual. 

`list ...args ` : Returns a Q-Expression of args. 

`head { ... }` : Returns the leading element of the Q-Expression. 

`tail { ... }` : Returns the Q-Expression with the head chopped off. 

`eval { ... }` : Turns a Q-Expression into an S-Expression and evaluates it. 

`join { ... } { ... } ... ` : Concatenates multiple Q-Expressions 

`join < string > < string > `: Concatenates strings. 

`!if { cond } { then } { else } ` : If condition is true, 'then' is executed. 'else' is executed otherwise. 

`===` : Compares anything, element by element. 

`print < string > ` : Prints a string to stdout. 

`load < filename as string > `: Executes the code for a given file. 

`error < string > `: Returns an error with the given string as a message. Can be used to define custom exceptions. 

---

##### Bindings 


There's two ways to define a binding (or a variable) in this language, based on what scope you want to shoot it to. 

To update the global scope, do 

` def { binding name }  < binding value > `

To update the local scope (which, outside of a function, is also the top scope), do 

` = { binding name } < binding value > `

To check the value of a binding (at least in the console) you can just enter `< binding name >`, which returns whatever value or expression associated with the binding. 

The difference between these two methods of defining a binding are more significant on the back end of things. If you're playing around in the REPL loop, I would recommend just using `def`, as it feels a bit more clear. If you want to write your own programs in the language, the definition becomes more important. Defining a binding inside of a function using `=` will create a local variable scoped by that function. Definining a binding inside of a function with `def` is equivalent to defining a global constant inside of the function body, and the value will be accessible outside of the local scope. 

In either case, bindings persist for the duration of the program, and can be updated or modified at will. This also means that you can alter the standard library definitions, so be cautious about over-writing things unintentionally.

---

##### Conditionals / Comparisons  

---

##### Strings 

---

##### Function Definition 

---

##### Partial Function Definition

---

##### Loading Files 

Put in a word here about format. Enclose each 'statement', so to speak, in an S-Expression. That's how we denote a line of executible code. Comments are denoted by '::' 

---

## Under the Hood 

--- 

## Development Bits 

#### Current Bugs 

*To be wrung out before moving to To Do*

-  ~~Something is being freed too many times, throws a malloc bp. I think that it has to do with the free_env() function.~~ 
-  ~~Verify that function definition works.~~ 
-  Track down the segfaults associated with variable argument functions 
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

