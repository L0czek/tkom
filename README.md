# Yet another custom language compiler
This is a compiler utilizing custom lexer, recursive descent parser and  LLVM as backend with JIT support. The language is a limited, almost completely useless (partly due to lack of structs support), version of C with some rust like syntax. It supports int variables, pointers to int's and string literals (as wchar_t*). As for statements my language has all the classic conditional and loop statements, it has python like if-elif-else statement (though as a result of me being lazy it lacks ternary operator), while and for loop. The main feature of this language which makes this project not entirely useless is the ability to call functions from libc by providing their declaration with `extern` keyword.  Unfortunately I haven't implemented variable arguments so calling functions like printf will probably result in a disaster.

## Building
```sh
mkdir build
cd build
cmake ..
make
```
## Usage
```sh
./rc --help
Allowed options:
  -h [ --help ]            produce help message
  -i [ --input-file ] arg  set input file
  -o [ --output-file ] arg set output file
  --jit                    execute compiled program
  --ir                     compile to llvm's IR
  --bc                     compile to llvm's bytecode
  -p [ --print-ir ]        print llvm's IR
```

### Running code (with JIT)
```sh
loczek@loczek-pc ~ $ ./rc --input-file=brainfuck.r --jit

Podaj program brainfucka: ++++++++++[>+++++++>++++++++++>+++>+
<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.
Hello World!
```

### Printing LLVM's assembler (IR)
```sh
loczek@loczek-pc ~ $ ./rc --input-file=brainfuck.r --print-ir

; ModuleID = 'top'
source_filename = "top"
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-
f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"
target triple = "x86_64-unknown-linux-gnu"
@0 = common global i32* null
@1 = common global i32* null
@2 = common global i32 0
@3 = common global i32 0
@4 = private unnamed_addr constant [17 x i8] c"i\00\00\00p\00\00\00
\00\00\00\00\00\00\00\00", align 1
@5 = private unnamed_addr constant [9 x i8] c"\0A\00\00\00\00\00\00\00\00",
align 1
@6 = private unnamed_addr constant [21 x i8] c"p\00\00\00t\00\00\00r\00\00\00
\00\00\00\00\00\00\00\00", align 1
@7 = private unnamed_addr constant [9 x i8] c"\0A\00\00\00\00\00\00\00\00",
align 1
```

### Compiling to LLVM's assembler and later to binary using clang or running with lli
```sh
loczek@loczek-pc ~ $ ./rc --input-file=brainfuck.r --output-file=code.ll --ir

loczek@loczek-pc ~ $ lli code.ll

Podaj program brainfucka: ++++++++++[>+++++++>++++++++++>+++>+
<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.
Hello World!
loczek@loczek-pc ~ $ clang code.ll -o bf

warning: overriding the module target triple with x86_64-pc-linux-gnu [-
Woverride-module]
1 warning generated.
loczek@loczek-pc ~ $ ./bf

Podaj program brainfucka: ++++++++++[>+++++++>++++++++++>+++>+
<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.
Hello World!
```

### Compiling to LLVM's bytecode and later to binary using clang or running using llc 
```sh
loczek@loczek-pc ~ $ ./rc --input-file=brainfuck.r --output-file=code.bc --bc

loczek@loczek-pc ~ $ lli code.bc
Podaj program brainfucka: ++++++++++[>+++++++>++++++++++>+++>+
<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.
Hello World!

loczek@loczek-pc ~ $ clang code.bc -o bf
warning: overriding the module target triple with x86_64-pc-linux-gnu [-
Woverride-module]
1 warning generated.

loczek@loczek-pc ~ $ ./bf
Podaj program brainfucka: ++++++++++[>+++++++>++++++++++>+++>+
<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.
Hello World!
```

## Code examples
### Printing string

```sh
extern fn putwchar(chr : int) -> int;

fn putstr(str: string) -> int {
    let i=0 : int;
    while str[i] {
        putwchar(str[i]);
        i = i + 1;
    }
    return i;
}

fn main() -> int {
    putstr("OKI");
    return 0;
}
```

### Printing numbers
```sh
extern fn putwchar(chr : int) -> int;

fn putint(num : int) -> int {
    if num == 0 {
        return 0;
    }
    let digit = num % 10 : int;
    putint(num / 10);
    putwchar(digit + 48);
    return 0;
}

fn putstr(str: string) -> int {
    let i=0 : int;
    while str[i] {
        putwchar(str[i]);
        i = i + 1;
    }
    return i;
}

fn main() -> int {
    for i in 1..20 {
        putint(i);
        putstr("\n");
    }
    return 0;
}
```

### And yet another brainfuck interpreter
```sh
extern fn putwchar(chr : int) -> int;
extern fn malloc(size : int) -> int*;
extern fn getchar() -> int;
extern fn free(ptr : int*) -> int;
extern fn memset(ptr : int*, val : int, size : int) -> int*;

let code : int*;
let ram : int*;
let ptr = 0 : int;
let ip = 0 : int;

fn putstr(str: string) -> int {
    let i=0 : int;
    while str[i] {
        putwchar(str[i]);
        i = i + 1;
    }
    return 0;
}

fn readstr(ptr : int*, size : int) -> int* {
    for i in 0..size {
        let ch=getchar() : int;
        if ch == 10 {
            return ptr;
        }
        ptr[i] = ch;
    }
    return ptr;
}

fn read_char() -> int {
    ram[ptr] = getchar();
    return 0;
}

fn print_char() -> int {
    putwchar(ram[ptr]);
    return 0;
}

fn inc() -> int {
    ram[ptr] = ram[ptr] + 1;
    return 0;
}

fn dec() -> int {
    ram[ptr] = ram[ptr] - 1;
    return 0;
}

fn next() -> int {
    ptr = ptr + 1;
    return 0;
}

fn prev() -> int {
    ptr = ptr - 1;
    return 0;
}

fn go_to_loop_beg() -> int {
    ip = ip - 1;
    let count = 1 : int;
    while count > 0 {
        if code[ip] == 91 {
            count = count - 1;
        } elif code[ip] == 93 {
            count = count + 1;
        }
        ip = ip - 1;
    }
    return 0;
}

fn go_to_loop_end() -> int {
    if ram[ptr] == 0 {
        ip = ip + 1;
        let count = 1 : int;
        while count > 0 {
            if code[ip] == 91 { # '['
                count = count + 1;
            } elif code[ip] == 93 { # ']'
                count = count - 1;
            }
            ip = ip + 1;
        }
        ip = ip - 1;
    }
    
    return 0;
}

fn execute() -> int {
    while code[ip] {
        let opcode = code[ip] : int;
        if opcode == 44 { # ','
            read_char();
        } elif opcode == 46 { # '.'
            print_char();
        } elif opcode == 43 { # '+'
            inc();
        } elif opcode == 45 { # '-'
            dec();
        } elif opcode == 62 { # '>'
            next();
        } elif opcode == 60 { # '<'
            prev();
        } elif opcode == 91 { # '['
            go_to_loop_end();
        } elif opcode == 93 { # ']'
            go_to_loop_beg();
        } else {
            putstr("Invalid opcode ");
            putwchar(opcode);
            return -1;
        }
        ip = ip + 1;
    }
    return 0;
}

fn main() -> int {
    putstr("Podaj program brainfucka: ");
    let size = 32768 : int;
    ram = malloc(size);
    code = malloc(size);
    memset(ram, 0, size);
    memset(code, 0, size);
    readstr(code, size);
    execute();
    free(ram);
    free(code);
    return 0;
}
```
