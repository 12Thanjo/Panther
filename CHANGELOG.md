# Changelog

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
	- underscores as separators (`10_000`)
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