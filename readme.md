g# Skeem #
## Description ##
Skeem is a toy Lisp interpreter implemented in C, which can be seen as the next educational step
from a lisp interpreter I wrote in [Python](https://github.com/randomhajile/lisp-simulacrum).

This is by no means a production quality implementation, and there's a strong chance it never will be.
The goal of the project was to learn some C, which I didn't know much of when I started (I often feel like I still don't...),
and to make a simple interpreter that someone with a modest amount of background knowledge could understand in a weekend.
Most of the inspiration was drawn from

* [Dragon Book](http://www.amazon.com/Compilers-Principles-Techniques-Tools-2nd/dp/0321486811/ref=sr_1_1?ie=UTF8&qid=1447383636&sr=8-1&keywords=compilers+principles+techniques+and+tools)
*The* reference for all things language implementation.
* [K&R](http://www.amazon.com/Programming-Language-Brian-W-Kernighan/dp/0131103628/ref=sr_1_1?ie=UTF8&qid=1447380884&sr=8-1&keywords=the+c+programming+language):
*The* reference for learning the basics of C and how to think about the language.
* [Writing Compilers and Interpreters](http://www.amazon.com/Writing-Compilers-Interpreters-Applied-Approach/dp/0471555800/ref=sr_1_3?s=books&ie=UTF8&qid=1447298677&sr=1-3&keywords=writing+compilers+and+interpreters) (1st edition):
This may not be the right book if you know nothing about language implementation, [Dragon Book](http://www.amazon.com/Compilers-Principles-Techniques-Tools-2nd/dp/0321486811/ref=sr_1_1?ie=UTF8&qid=1447383636&sr=8-1&keywords=compilers+principles+techniques+and+tools)
is the go to reference there. But, if you already know some theory, but not much C, then this will show you how to effectively use C in the context of langauge implementation.
The tokenizer of Skeem was heavily inspired by the treatment in this book.
* [TinyScheme](http://tinyscheme.sourceforge.net/):
A relatively easy to understand Scheme implementation in C.
I also recommend looking at MiniScheme, available on TinyScheme's [download](http://tinyscheme.sourceforge.net/download.html) page, which is a bit simpler. The
op codes for Skeem are mostly copied directly from MiniScheme.


## Syntax ##
The syntax is some combination of Scheme and Common Lisp, leaning more toward Scheme. For example, basic function
definition look like:
```scheme
(define (square x)
  (* x x))
```

More complicated functions, however, borrow some syntax from Common Lisp:
```scheme
(define (foo x &optional y)
  (if (null? y)
    x
    y))
```
and
```scheme
(define (foo x &rest rest)
  (if (null? rest)
    x
    rest))
```
For optional arguments, the default is `nil`, someday I'd like to add user-supplies
defaults.

## Todo ##
* Fix known segfaults on some bad syntactic forms, e.g. (define (+ 1 2 3)).
* Implement `cond` - You could argue that it's not really necessary, but I'd probably disagree with you.
* Code cleanup - While trying to figure out good ways to do a few things (e.g. binding values to function parameters) the code got a bit wet. It's not super concerning, but I'd like to clean some of that up.
* Remove simplifying limitations - There are caps on things like string and identifier length. This was done to help speed up building the project, but should be removed eventually.
* Bignums - The interpreter eliminates tail calls, but uses C's builtin int type so we get overflow errors.
* Macros - Generally considered the Holy Grail of Lisp programming. I'll probably go for Common Lisp style macros.
* Tests - Write a suite of simple tests.
* Documentation - Most of the comments are just for my own benefit and I should thoroughly document any tricky spots.
* Bug squashing - Obviously, bugs need to be squashed as they appear.
