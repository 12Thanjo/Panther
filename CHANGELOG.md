# Changelog

### v0.27.0
- Changed the name mangling prefix from "P" to "PTHR"
- Added structs
	- members can be `var` or `def`
- Added accessor (`.`) operator to access struct members
- added `@__printBool`
	- This existence of this is just temporary
- Added checking for usage of `uninit` in not an assignment operation

### v0.26.1
- Added parsing of function parameter packs
- Fixed fatal error caused by not properly checking that an identifier can be called like a function
- Fixed function calls not being properly checked by semantic analysis

### v0.26.0
- Added the following intrinic functions:
	- `@equalInt(Int read, Int read) -> Int`
	- `@notEqualInt(Int read, Int read) -> Int`
	- `@lessThanInt(Int read, Int read) -> Int`
	- `@lessThanEqualInt(Int read, Int read) -> Int`
	- `@greaterThanInt(Int read, Int read) -> Int`
	- `@greaterThanEqualInt(Int read, Int read) -> Int`
	- `@equalUInt(UInt read, Uint read) -> UInt`
	- `@notEqualUInt(UInt read, Uint read) -> UInt`
	- `@lessThanUInt(UInt read, Uint read) -> UInt`
	- `@lessThanEqualUInt(UInt read, Uint read) -> UInt`
	- `@greaterThanUInt(UInt read, Uint read) -> UInt`
	- `@greaterThanEqualUInt(UInt read, Uint read) -> UInt`
	- `@equalBool(Bool read, Bool read) -> Bool`
	- `@notEqualBool(Bool read, Bool read) -> Bool`
	- `@logicalAnd(Bool read, Bool read) -> Bool`
	- `@logicalOr(Bool read, Bool read) -> Bool`
	- `@logicalNot(Bool read) -> Bool`
- Added the following operators:
	- `==`
	- `!=`
	- `<`
	- `<=`
	- `>`
	- `>=`
	- `&&`
	- `||`
	- `!`

### v0.25.0
- Fixed formatting for building documentation
- Added `alias` statements

### v0.24.1
- Added implicit conversion for infix operators
- Fixed an implicit conversion error not properly reporting
- Fixed some multiply operators calling the incorrect functions

### v0.24.0
- Added the UInt type
- Added implicit conversion for integral literals
- Added the intrinsic function `@__printUInt(UInt read)` and `@__printSeparator()`
	- This existence of this is just temporary
- Added the following intrinsic functions:
	- `@addInt(Int read, Int read) -> Int`
	- `@addUInt(UInt read, UInt read) -> UInt`
	- `@addWrapInt(Int read, Int read) -> Int`
	- `@addWrapUInt(UInt read, UInt read) -> UInt`
	- `@subInt(Int read, Int read) -> Int`
	- `@subUInt(UInt read, UInt read) -> UInt`
	- `@subWrapInt(Int read, Int read) -> Int`
	- `@subWrapUInt(UInt read, UInt read) -> UInt`
	- `@mulInt(Int read, Int read) -> Int`
	- `@mulUInt(UInt read, UInt read) -> UInt`
	- `@mulWrapInt(Int read, Int read) -> Int`
	- `@mulWrapUInt(UInt read, UInt read) -> UInt`
	- `@divInt(Int read, Int read) -> Int`
	- `@divUInt(UInt read, UInt read) -> UInt`
	- `@negateInt(Int read) -> Int`
- Added the following operators:
	- `+`
	- `+@` (addition with wrapping)
	- `-` (both negate and subtraction)
	- `-@` (subtraction with wrapping)
	- `*`
	- `*@` (multiplication with wrapping)
	- `/`
	

### v0.23.0
- Added function overloading
- Fixed fatal errors with using functions by explicitly disallowing it (only temporary)
- Fixed implicit conversions for function arguments
- Added checking for multiple functions with the attribute `#export` with the same name
- Temporarily disallowed functions with the attribute `#export` from having parameters
- Organized build-system
	- renamed CLI to pthr
	- added groupings
	- made the compiler the default start project
- Added the `#export` attribute for variables

### v0.22.1
- Added some missing uses of imports
	- `def sub_import = some_import.sub_import;` at local scope (at global scope will be supported eventually)
	- `def sub_import = @import("some_import.pthr").sub_import;`

### v0.22.0
- Added tokenizing and parsing of type `String`
- Fixed literal strings and literal chars not being printed correctly in the `PrintAST` output target
- Added checking if type is a literal float or literal char
	- (not supported yet, so it errors)
- Fixed intrinsic function calls as expressions causing a fatal error
- Fixed AST printer adding `[UNREACHABLE]` after a function call
- Added the `#pub` intrinsic
- Added the intrinsic function `@import(String read)`
- Fixed empty string / char literals causing a fatal error
- Added attributes for variable declarations
- Added error if discarding the return value of a function

### v0.21.4
- Removed being able to set global variables to be value of another variable
	- this is only temporary
- Added order-independant declaration (in global scope)
- Added variable type inference

### v0.21.3
- `read` parameters are no longer marked as `readonly` in LLVM IR automatically
	- The intention is to add this back in some optimization pass
- Added checking to prevent taking the address of an intrinsic

### v0.21.2
- Added the intrinsic function `@__printInt(Int read)`
	- This existance of this is just temporary
- Added warning if a `write`  parameter is known to never be written to

### v0.21.1
- Added a semantic analysis check to make sure that arguments to write parameters are mutable
- Added more parameter marking in LLVM IR
	- `read` parameters marked as `readonly`
	- `nonnull`
	- `write` parameters marked as `noalias`
	- `dereferenceable`

### v0.21.0
- Fixed printing of AST not having an arrow for function attributes
- Added function parameters
	- `read` and `write` parameters work
	- `in` is not supported yet
- Fixed call to breakpoint when type of return expression was incorrect
- Added implicit const pointer conversion for return statements
- Fixed incorrect LLVM IR code being generated for functions that return `Void` without an explicit `return` in all control paths

### v0.20.2
- Added implicit conversion from a non-const-pointer to a const-pointer

### v0.20.1
- Fixed assert firing when using a function as a value
- Fixed breakpoint firing when funciton marked with `#entry` does not return `Int`

### v0.20.0
- Added `def` variable declaration
- Fixed local variables being initialized with the value `uninit`
- Global variables can no longer be initialized with the value `uninit`
- Added const pointers (add `|` to the pointer)
- Fixed global variables being initialized with an `addr` expression
- Made it illegal to initialize a global variable with an dereference expression

### v0.19.0
- Added `unreachable` statements
- Added detection for unreachable code after `return` (and `unreachable`) statements inside conditionals
- Fixed `return` statements in conditionals causing LLVM to error
- Changed `@__printHelloWorld` from using `printf` to using `puts`
- Added `puts` to the list of functions that cannot be used as export names
- CLI now only wait's for user input to exit when compiling with MSVC when not doing a ReleaseDist build

### v0.18.1
- Added tokenizing of string and char literals
	- supports escape characters: `\0`, `\a`, `\b`, `\t`, `\n`, `\v`, `\f`, `\r`, `\'`, `\"`, and `\\`
- Improved messaging location of multiline objects

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
- Added output to Executable
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
- Added `#export` function attribute
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