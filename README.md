# Godunov/GRP scheme for Lagrangian/Eulerian hydrodynamics
What is it?
-----------
This is a implementation of fully explict forward Euler scheme for single/multi-fluid Euler equations of motion on Lagrangian/Eulerian coordinate.

How to use it?
-----------
Programer's Guide can be found in folder 'doc'.

It is made by Doxygen and LaTeX,

Run the following command on the terminal.

```
cd src/*
doxygen Doxyfile
cd doc/*/latex
make
cd doc/*/Specification
xelatex Specification.tex
```

Open 'doc/*/html/index.html' in a browser to view the specific instructions of this program.

Debugging tools
---------
gprof, gcov, lcov, Valgrind, Cppcheck, gprof2dot.

Licensing
---------
GNU Lesser General Public License v3.0 or later.

Please see the file called LICENSE.

Contacts
--------
If you want more available support for this program, please send an email to  [xinlei@cugb.edu.cn](mailto:xinlei@cugb.edu.cn).

Copyright
--------
Copyright © 2022 ximlel.

> Part of this code is modified from the provision of Zhifang Du.
> The source code in the book "C Interfaces and Implementations" by David Hanson is used.