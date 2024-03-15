# Building

This file is not complete.


## Building the LLVM Project
You can do this anywhere on your PC.

`mkdir LLVM`
`cd LLVM`
`mkdir build`
`git clone https://github.com/llvm/llvm-project.git --depth=1`


### Windows:
In this directory create `setup_llvm.bat`, and fill it with this:

```bat
cd ./build

cmake "../llvm-project/llvm" ^
  -G "Visual Studio 17 2022" ^
  -DCMAKE_INSTALL_PREFIX="../llvm-project/llvm/build-output" ^
  -DCMAKE_PREFIX_PATH="../llvm-project/llvm/build-output" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DLLVM_ENABLE_PROJECTS="lld;clang" ^
  -DLLVM_ENABLE_LIBXML2=OFF ^
  -DLLVM_ENABLE_ZSTD=OFF ^
  -DLLVM_INCLUDE_UTILS=OFF ^
  -DLLVM_INCLUDE_TESTS=OFF ^
  -DLLVM_INCLUDE_EXAMPLES=OFF ^
  -DLLVM_INCLUDE_BENCHMARKS=OFF ^
  -DLLVM_INCLUDE_DOCS=OFF ^
  -DLLVM_ENABLE_BINDINGS=OFF ^
  -DLLVM_ENABLE_OCAMLDOC=OFF ^
  -DLLVM_ENABLE_Z3_SOLVER=OFF ^
  -DLLVM_TOOL_LLVM_LTO2_BUILD=OFF ^
  -DLLVM_TOOL_LLVM_LTO_BUILD=OFF ^
  -DLLVM_TOOL_LTO_BUILD=OFF ^
  -DLLVM_TOOL_REMARKS_SHLIB_BUILD=OFF ^
  -DLLVM_BUILD_TOOLS=OFF ^
  -DCLANG_BUILD_TOOLS=OFF ^
  -DCLANG_INCLUDE_DOCS=OFF ^
  -DLLVM_INCLUDE_DOCS=OFF ^
  -DCLANG_TOOL_CLANG_IMPORT_TEST_BUILD=OFF ^
  -DCLANG_TOOL_CLANG_LINKER_WRAPPER_BUILD=OFF ^
  -DCLANG_TOOL_C_INDEX_TEST_BUILD=OFF ^
  -DCLANG_TOOL_ARCMT_TEST_BUILD=OFF ^
  -DCLANG_TOOL_C_ARCMT_TEST_BUILD=OFF ^
  -DCLANG_TOOL_LIBCLANG_BUILD=OFF ^
  -DLLVM_USE_CRT_RELEASE=MT ^
  -DLLVM_BUILD_LLVM_C_DYLIB=NO

if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
cmake --build . %JOBS_ARG% --target install
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
```

Run `setup_llvm.bat`. This will take a very long time

### Other Platforms:
Not supported yet


## Adding LLVM to the Panther Project:
Once you've successfully built the LLVM Project, you will need to go into `./llvm-project/llvm/build-output`. Copy `include` and `lib` into the directory `Panther/libs/LLVM` (the libs directory already exists, but you need to create the LLVM directory inside).


