# Changelog

### v0.18.0
- Added conditional statements
	- `if`
	- `else if`
	- `else`

### v0.17.0
- Added `@breakpoint` intrinsic
- Panther functions (not marked with `#export`) now use the fast calling convention

### v0.16.0
- Reorganized part of the build system
- Changed debug working directory to `/testing`
- Added output to executable
- Added default file output naming

### v0.15.0
- Added the intrinsic function `@__printHelloWorld()`
	- This existance of this is just temporary

### v0.14.2
- Fixed printing of AST for postfix operators
- Fixed "crashing" when doing semantic analysis of intrinsics
- Added marking functions as not throwing (in LLVM IR)

### v0.14.1
- Added location printing for function calls
- Added location printing for return statements

### v0.14.0
- Added pointer types
- Added the `addr` keyword
- Added the dereference (`.^`) operator 

### v0.13.1
- Fixed `copy` expressions in globals
- Fixed bug where compiler sometimes incorrectly errored that the entry function returned the incorrect type

### v0.13.0
- Added `copy` expressions

### v0.12.0
- Added Panther runtime
- Added `#entry` function attribute

### v0.11.0
- Added parenthesis expressions
- Added function call expressions and statements
- Added printing of the current output target in verbose mode
- Added semantic analysis checking for literal, ident, and uninit expressions

### v0.10.0
- Added assigning new values to variables

### v0.9.0
- Added return statements
- Added `#extern` function attribute
- Added tokenizing of intrinsics

### v0.8.0
- Made global variables have private linkage
- Semantic Ananlysis now catches setting global variables to variables with value uninit
- Fixed setting variables to identifiers

### v0.7.0
- Added build instruction for LLVM
- Improved Semantic Analysis
- Added uninit expression
- Added output to LLVMIR
- Added output to object file

### v0.6.0
- Added identifier expressions

### v0.5.0
- Added parsing and semantic analysis of functions

### v0.4.1
- Added messaging for location of variable already defined (when trying to use the sane identifier)

### v0.4.0
- Added Semantic Analysis of variables

### v0.3.0
- Added parsing
	- variable declarations
	- basic types
	- literals
- Added printing of the AST

### v0.2.0
- Added foundation for parsing
- Added printing of tokens

### v0.1.0
- Added basic tokenization

### v0.0.0
- initial creation
- setup of build system