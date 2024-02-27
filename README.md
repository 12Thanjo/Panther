![Logo](art_assets/Logo_scaled.png)
# Panther
Statically typed, compiled, high-performance, general-purpose programming language.

### Notes:
- This compiler is currently super early into development, so it doesn't really do much yet.
- The logo may change in the future, but for now it's good enough


## Goals:
These are the goals that I wish to achieve with this language / compiler. In no particular order:
- Fast compile times
	- fast enough to compile all source code at once (create just one translation unit)
- Nice, readable, and helpful error messages
- Compiler helps you find mistakes (for example, nodiscard by default)
- Zero-cost abstractions
- Fast runtime (performance on-par with languages like C++)
- Language helps you catch mistakes without getting in the way (so nothing like Rust's borrow-checker)
- Seamless interop with C code (like Zig)
- Nice, readable code
	- No preprocessor
- Enjoyable to use
- Run arbitrary code at compile time (like Jai)
- Improved optimizability
	- Give the compiler more information about language constructs (without being verbose)
	- Attempt to run as much code at compile time as possible (yes, I'm aware of the halting problem)
- The build system is in Panther (like Zig and Jai)
- Compile-time reflection


## Long Term Goals:
These are the goals I have that are for after a 1.0.0 release:
- Rich and high-performance standard library
- Seamless interop with C++
	- May be impractical or impossible as Panther doesn't have features like inheritance or exceptions
- Self-hosted compiler (meaning, the Panther compiler is written in Panther)
- Panther debugger
- Package manager


## Downloading:
`git clone https://github.com/12Thanjo/Panther.git --recursive`


## Updates:
List of changes for each version can be found [here](CHANGELOG.md).



