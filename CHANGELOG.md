# Changelog

### v0.11.0
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