# JIT Calculator
World's most useless calculator, but it's JIT. 

> [!WARNING]
> This project was made completely for educational purposes.

## Example usage

Calculates expressions written in prefix form (NOT Polish Notation):

```
> -1 + 2
Computed value: 1
> -2 + 3 - 4
Computed value: -3
> + 123 - 23
Computed value: 100
```

Currently only sum and subtraction operations are supported. More advanced features are on the way. 

## Quick Start
Only Linux + x86-64 is supported.
```
$ make 
$ ./a.out
```

## Implementation details
After expressions are converted to bytecode, ```get_jitfunc()``` iterates over each command, emmiting machine code. 
For each command, instruction of form `add rax, NUM` or `sub rax, NUM` are generated.

Each JIT-compiled block of code starts with ```xor rax, rax```, and ends with `ret`. All the math calculations are then happening in the `rax` register (which will implicitly store our function's return value). 

After machine code is generated, executable page is being mapped using `mmap`, returning a `void*` where the machine code is going to be copied. We can then freely cast mapped page to a function, and call it normally.
