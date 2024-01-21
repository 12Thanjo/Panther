# Changelog

### v0.0.19
- Fixed all build warnings (in MSVC)
- Added error messaging if function errors are empty (`<>`)
- Added error message info for missing identifier in funciton return list
- Fixed error message for missing semicolon at end of variable declaration pointing to the wrong location
- Added the ability to make structs public
- Added parsing of typedef and alias statements

### v0.0.18
- Added parsing of the `this`  parameter
- Added parsing of operator overloading

### v0.0.17
- Added parsing of structs
- Fixed use of literal 0 causing a divide by 0 error
- Added ability for variables to not have defined values (for use as struct members)
- Added parsing of try-catch

### v0.0.16
- Added parsing of multiple assignment
- Added parsing of conditionals (`if`, `else`, `else if`)
- Added parsing of while loops
- Added parsing of return / throw statements

### v0.0.15
- Simplified operator precedences
- changed pointer to `^`
	- dereference operator is now `a.^`
- Added parsing of function calls
- Added parsing of expression statements
- Added parsing of anonymous blocks
- Added parsing of assignment statements

### v0.0.14
- Fixed `a + b + c` from being parsed like `a + (b + c)`
- Added parsing of accessing operators
	- accessor (`a.b`)
	- dereference (`a.*`)
	- unwrap (`a.?`)
	- index (`a[b]`)

### v0.0.13
- Made function output definitions (returns and errors) their own AST node
- Added parsing of function types (function pointers)

### v0.0.12
- Removed function captures due to changing design decision
	- should be added back when the design is finalized
- Added optional type operator (`?`) to tokenizing
- Added parsing of complex types
	- pointer qualifier (`*`)
	- const pointer qualifier (`|`)
	- optional qualifier (`?`)
	- Arrays

### v0.0.11
- Adding parsing of function definitions

### v0.0.10
- Added parsing of expression operators
- Added parsing of parenthesis enclosed expressions
- Added Printer class to manage logging
	- ability to disable colors

### v0.0.9
- Added the description.pthr file
- Added tokenizing of the builtin `String` and `Char` types
- Added parsing literals, identifiers, `null`, and `this`

### v0.0.8
- Added checking for EOF while parsing variable declarations
- Improved error messages (now points to section instead of just one point)
- Fixed assert firing incorrectly during tokenizing

### v0.0.7
- Added more keywords / types to the tokenizer
- Added grammar description file
- Added basic structure of Parser
- Added console printing of AST (for debug purposes)

### v0.0.6
- Added tokenizing of operators
- Added tokenizing of many more keywords
- Added tokenizing of builtin types

### v0.0.5
- Added tokenizing of number literals
	- integers
	- floating-point 
	- binary, octal, and hex
	- exponents (positive and negative)
	- underscores as separators (for example: `10_000`)
	- checking that the number fits in a `UI64` or `F128` for integers and floating-points respectively

### v0.0.4
- Reordered README sections
- Updated goals in README
- Added logo / icon
	- Added logo to README

### v0.0.3
- Added to Tokenizing
	- Intrinics
	- Attributes
	- bool literals
	- string/char literals
	- Keywords
- Added to README (goals, etc.)

### v0.0.2
- Added Tokens
- Added Tokenizer
	- identifiers
	- whitespace
	- comments
- Added compiler error messages

### v0.0.1
- Addes SourceManager
- Added CharStream

### v0.0.0
- initial creation
- setup of build system