# Safiniea 
##### *A little lispy language.*
---

## Project Overview 

This project is based on orangeduck's fantastic [buildyourownlisp][r1]. My hope was to build a fun, flavorful little programming language in C. Safiniea supports a relatively primitive set of operations, and is nowhere near production-class. However, it supports a number of features uncommon in other languages, such as the ability to manipulate code as data, and partial function definition. This project is very much *in progress*, and the current source is littered with bugs and clutter. If it strikes your interest, stay tuned! Tour the program! Visit orangeduck's site! Use the fantastic mpc library and start building things! Huzzah! 

---

## Quick Start Guide 

Safiniea is currently a command-line REPL loop that only caches data for the duratino of a single session. The current code will print not only the interactions between the user and the language, but a whole plethora of parsing metadata as well as a smattering of debug statements. As mentioned, *in progress...*. However, even from this highly developmental state, you can have fun with the loop and fart around with various operations. 

#### A note about syntax 

Safiniea uses a Polish syntax, so get a sense of what you want to do before you type it. In other words, `x + y` is `+ x y`, `8 * (7 + 3 (8 - 5))` is `* 8 (+ 7 (* 3 (- 8 5)))`, etc. It can be a bit jarring at first, but I was surprised to find that it wears well. Obviously, Polish syntax was chosen to make the project as simple as possible, and while annoying at first, it feels quirky and cute and neat at this point in the project.  

#### Arithmetic 

Safiniea currently has a single data type: the humble integer. 
Safiniea supports addition, subtraction, multiplication, division, etc. 
`min` and `max` return the minimum or maximum value of an arbitrary list of arguments. 

*Modular arithmetic and exponentiation are currently in-progress*.  

#### Built-In Functions  

#### Bindings  

#### Conditionals 

#### Strings 

#### Function Definition 

#### Partial Function Definition

---

## Under the Hood 

--- 

## Development Bits 

#### Current Bugs 

*To be wrung out before moving to To Do*

- [ ] Something is being freed too many times, throws a malloc bp. I think that it has to do with the free_env() function. 
- [ ] Verify that function definition works. 
- [ ] Verify that partial function definition works 
- [ ] Verify that currying-uncurrying works 
- [ ] Verify all arithmetic operations 
- [ ] Verify Q-expr operations (head, tail, list) 
- [ ] Verify that bindings are preserved through a session 
- [ ] Verify that variable-argument functions work as expected 


#### To Do 

*High Priority*

- [ ] Do everything on the debug list.  

*Low Priority*

- [ ] Change linked list syntax to match C style (uniform, uniform)
- [ ] Continue to build out the README 
- [ ] Complete the program with full functionality 

#### Back Burner 

- [ ] Build a repl.it project so that the language can be demoed without cloning the repo 
- [ ] Build out the standard library 
- [ ] Prettify everything. 

[r1]: http://www.buildyourownlisp.com
