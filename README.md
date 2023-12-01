# Panther
Statically typed, compiled, high-performance, programming language.

## Goals:
These are the goals that I wish to achieve with this language / compiler. In no particular order:
- Fast compile times
	- fast enough to compile all source code at once (create just one translation unit)
- Nice, readable, and helpful error messages
- Zero-cost abstractions
- Fast runtime (Performance on-par with C++)
- Language helps you catch mistakes without getting in the way (so nothing like Rust's borrow-checker)
- Seamless interop with C code (much like Zig)
- Nice readable code
	- No preprocessor
- Enjoyable to use
- Run arbitrary code at compile time (like Jai)
- Improved optimizability
	- Give the compiler more information about language constructs (without being verbose)
	- Attempt to run as much code at compile time as possible (yes, I'm aware of the halting problem)


## Long Term Goals:
These are the goals I have that are for after a 1.0.0 release:
- Seamless interop with C++
	- May be impractical or impossible as Panther doesn't have features like inheritance or exceptions
- Self-hosted compiler (meaning, the Panther compiler is written in Panther)
- Panther debugger


### Note:
This compiler is currently super early into development, so it doesn't really do much yet.


## Updates:
List of changes for each version can be found [here](https://github.com/12Thanjo/Panther/blob/main/CHANGELOG.md).



