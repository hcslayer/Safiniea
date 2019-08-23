# Safiniea 
##### *A little lispy language.*
---

## Project Overview 

> [Quick Start Guide](#quick-start-guide)  
> [Example Code](#example-code)  
> [Arithmetic](#arithmetic)
> [Built-In Functionality](#built-in-functions)  
> [Standard Library](#standard-library)  
> [Bindings](#bindings)  
> [Conditionals](#conditionals-and-comparisons)  
> [Strings](#strings)  
> [Loading Code](#loading-files)  


This project is based on orangeduck's fantastic [buildyourownlisp][r1]. My hope was to build a fun, flavorful little programming language in C. 

Safiniea is based on a small core of data types and operations: numbers (integers), strings, executibles (S-Expressions), non-executibles (Q-Expressions), errors, and booleans. This is nowhere near a production class language, but it has dignity and spunk, and it was a true joy to build. 

Safiniea also supports a number of features uncommon in other languages, such as the ability to manipulate code as data, partial function definition, variable argument functions, and more. This project is very much *in progress*, and the current source is *proudly* littered with bugs and clutter. If it strikes your interest, stay tuned! Tour the program! Visit orangeduck's site! Use the fantastic [mpc library][r2] and start building your own lispy things! Huzzah! 

---

## Quick Start Guide 

Safiniea can be run in two ways, both from the CLI. Simply running `prompt` or `./prompt` will fire up a console session, where you can type your Saf code into stdin and play around. 

Alternatively, you can do `./prompt file1 file2 ... ` to load files into the program. Each file will be parsed and evaluated in the sequence provided, and any and all outputs will be logged to the console. When writing your `.saf` source code files, be sure to wrap all statements in `( ... )`, so that they will be properly parsed and evaluated. Otherwise, the parser will yell at you, or worse, silently betray you. 

In Safiniea, there are two main types of expressions: S `( ... )` and Q `{ ... } `. S-Expressions represent, more or less loosely, evaluable statements, while Q-Expressions are not evaluated. A single operator followed by an appropriately-formatted list of operands does not require padding in parentheses, however, more complex statements and compound expressions will likely use a combination of S-expressions, Q-expressions, and naked argument lists. 

All strings must be enclosed in double inverted commas `""`. 

Comments begin with double colons `::` and are terminated by the first newline character encountered.  

#### A quick note about syntax 

Safiniea uses reverse Polish syntax for all operations. It may be a bit unfamiliar at the onset, but I've found that it wears well. For the user unfamiliar with reverse Polish syntax, the general gist is that `<op1> <operator> <op2>` becomes `<operator> <op1> <op2> ...`

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

#### Arithmetic 

Safiniea currently has a single numeric data type: the humble integer. 
Safiniea supports addition, subtraction, multiplication, division, etc. 
`min` and `max` return the minimum or maximum value of an arbitrary list of arguments. 

Make note that if you pass more than two operands to a binary operator, the result will be evaluated pairwise. That is,  
`* a b c ... ` returns `(a * b) * c) * ... `

Most arithmetic in Safiniea is relatively intuitive. Zero division throws errors, and you can't (theoretically) do anything that a calculator wouldn't let you do.   

---

#### Built In Functions  
The following functions are hard-coded into the program itself. The standard library builds on this core set of builtins.

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

#### Standard Library 

The standard library, which is obtained by doing `load "stdlib.saf"` adds some additional quirky functions, about a third of which are actually useful! A quick summary of these follows, though the reader is encouraged to explore the stdlib functions *not* included on this list. 

`fun { <function name> arg1 arg2 ... } { <function body> } ` gives us a cleaner and more concise way to define functions. Note that the first Q-expression passed to `fun` places the function name as the first argument, followed by the formal arguments for the function itself. The function body is simply the code to be executed each time the function is called. 

`do <expr1> <expr2> ... <expr n>` evaluates `n` expressions in a sequence, and then returns the result of the final one. 

`len <list>` returns the length of a list. 

`caboose <list>` returns the last element in a list 

`makeset n <list>` makes a new list from the first `n` elements of the list. 

`prune n <list> ` returns a new list, with the first `n` elements of the original list removed. 

`forEach fcn <list> ` applies fcn to each element of the list, and returns a list of the evaluated results. 


---

#### Bindings 


There's two ways to define a binding (or a variable) in this language, based on what scope you want to shoot it to. 

To update the global scope, do 

` def { binding name }  <binding value> `

This is equivalent to defining a `const` or a `global`, and the binding will be accessible in any scope, provided that it isn't overridden by a local variable of the same name. 

To update the local scope (which, outside of a function, is also the top scope), do 

` = { binding name } <binding value> `

To check the value of a binding (at least in the console) you can just enter `<binding name>`, which returns whatever value or expression associated with the binding. 

The difference between these two methods of defining a binding are more significant on the back end of things. If you're playing around in the REPL loop, I would recommend just using `def`, as it feels a bit more clear. If you want to write your own programs in the language, the delineation between `=` and `def` becomes more important. Defining a binding inside of a function using `=` will create a local variable scoped by that function. Definining a binding inside of a function with `def` is equivalent to defining a global constant inside of the function body, and the value will be accessible outside of the local scope. 

In either case, bindings persist for the duration of the program, and can be updated or modified at will. This also means that you can alter the standard library definitions, so be cautious about over-writing things unintentionally.

---

#### Conditionals and Comparisons  

Safiniea supports a boolean type, but it's a bit more of an internal feature at the present moment. Soon, literal `true` and `false` values will be supported, but for now, the only way to get those values is by using some combination of `===, ==, !=, <, >, <= ...`

There's two different types of equality operators. The triple equals `===` is used for comparing non-integer types (think of it as a deep comparison), while the shallow comparison (arithmetic comparison) is handled by `==`. 

The rest of our friends, `> < != == <= >=`, operate as expected. Note that unlike some of the other arithmetic operators, these are strictly binary, and you will throw errors if more than two arguments are passed to any one. 

---

#### Strings 

Strings are any content enclosed in `"..."`. At the present, strings can be `join`-ed and `print`-ed. `print` simply returns the string to stdout, and `join` concatenates two (or more) strings and returns the result. 

---

#### Partial Function Definition

This is a neat feature of the language. See if you can figure out how it works... 

---

#### Loading Files 

If you aren't digging the command prompt, you can write Safiniea code and load it. Simply follow the `./prompt` command with the file(s) that you want to run. 
A couple of sample programs have been included to illustrate what this would actually look like. 

It's important to note that any statement you write in `.saf` code must be wrapped in `(...)`, otherwise wild errors abound. Comments, as mentioned earlier, span a single line and are denoted by `::`.

Other than these two guidelines, type away! I'm confident that you'll discover bugs I never did, or ways to break the language. *Honestly, please do*.   

--- 

## Dev Bits

#### Current Bugs 

*To be wrung out before moving to To Do*

-  ~~Something is being freed too many times, throws a malloc bp. I think that it has to do with the free_env() function.~~ 
-  ~~Verify that function definition works.~~ 
-  ~~Track down the segfaults associated with variable argument functions~~ 
-  ~~Verify that partial function definition works~~ 
-  ~~Verify that currying-uncurrying works~~ 
-  ~~Verify all arithmetic operations~~ 
-  ~~Verify Q-expr operations (head, tail, list)~~ 
-  ~~Verify that bindings are preserved through a session~~ 
-  ~~Verify that variable-argument functions work as expected~~ 
-  ~~Test and evaluate load functions~~ 
-  ~~Test standard library functions~~ 


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

