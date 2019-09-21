# Interpreters for Compatibility Libraries
                         
This directory contains interpreters for #F compatibility libraries. Each interpreter is implemented as a single .sf file and needs to be compiled and linked with the corresponding library, e.g.

```
$ sfc libs.sf ints.sf     # sfc produces 2 C files
$ gcc -lm libs.c ints.c   # gcc produces a.out (libs refers to math functions, so -lm may be needed)
```

## IntS, an Interpreter for LibS (Small) Library

IntS (see [ints.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/int/ints.sf)) allows interactive calling of LibS functions and use of LibS syntax forms, including `define-syntax` and `syntax-rules`. It provides full argument checking (LibS' own argument checking is limited to C asserts in debug mode).

There are some restrictions on the functionality available in IntS, compared to the #F code compiled with LibS:

  *  `read` uses LibS reader, limited to reading data types implemented in LibS
  *  fixnum (`fx`) and flonum (`fl`) - specific operations are not available
  *  C primitives and C code cannot be used
  *  `main` can be defined, but is not called by the interpreter

Since in IntS `load` is no longer a special form for separate compilation, it cannot be used to refer
to arbitrary #F files - only to files that can be dynamically loaded and executed by the interpreter. If you want to 'mask' your compile-time `load` forms from the interpreter, you may use the `#fload "myfile.sf"` abbreviated form, which the interpreter ignores.

Here is the list of IntS additions and things that behave differently between IntS and LibS:

  *  single-argument `eval` is available (macroexpands, compiles, and executes the argument)
  *  single-argument `expand` is available (macroexpands the argument)
  *  `load` is a procedure that dynamically loads and interprets Scheme code via `eval` 
  *  command-line file arguments are dynamically loaded 
  *  there is a traditional REPL (read-eval-print loop)


## IntM, an Interpreter for LibM (Medium) Library

IntM (see [intm.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/int/intm.sf)) is an extended version of IntS, based on LibM. It has the following additional functionality from R7RS small:

  *  support for bytevectors, with basic set of operations
  *  support for `define-record-type` records
  *  simple binary input from files and bytevectors
  *  simple binary output to files and bytevectors
  *  simple text output to/from strings
  *  `let-values`, `let*-values`, `define-values`, `make-parameter`, `parameterize`
  *  exceptions, errors, `guard` form
  *  current port access procedures are parameters
  *  additional r7rs math and port operations


## IntL, an Interpreter for LibL (Large) Library

IntL (see [intl.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/int/intl.sf)) allows interactive calling of LibL functions and use of LibS syntax forms. It provides full argument checking, `eval`, `load`, and interactive REPL.

There are some differences in the functionality available in IntL, compared to the #F code compiled with LibL:

  *  `read` uses LibL reader, limited to reading data types implemented in LibS
  *  `read` supports R7RS notation for circular structures, but `eval` and `load` reject them
  *  fixnum (`fx`) and flonum (`fl`) - specific operations are not available
  *  C primitives and C code cannot be used
  *  `main` can be defined, but is not called by the interpreter

Here is the list of IntL additions and things that behave differently between IntL and LibL:

  *  single-argument `eval` is available (macroexpands, compiles, and executes the argument)
  *  single-argument `expand` is available (macroexpands the argument)
  *  `load` is a procedure that dynamically loads and interprets Scheme code via `eval`
  *  command-line file arguments are dynamically loaded 
  *  there is a traditional REPL (read-eval-print loop)
  *  vectors are self-evaluating and don't need to be quoted
