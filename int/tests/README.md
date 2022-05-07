# Interpreters tests
                         
This directory contains tests interpreters for #F compatibility libraries. Tests
are named after the corresponding interpreters and need to be sent to the standard
input, e.g.:

```
$ ./ints < ints-tests.s
$ ./intm < intm-tests.s
$ ./intl < intl-tests.s
```

Tests are assembled from several test suites (Chibi Scheme* and Larceny** are the main sources).
Sources of the individual tests are attributed to their original authors inside the test files.

Tests that are not supposed to run on a given interpreter are commented out, so it is
easier to see what is supported and what is not.


----------------------------

\* available at https://github.com/ashinn/chibi-scheme

\*\* available at https://github.com/larcenists/larceny
