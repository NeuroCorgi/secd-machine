# SECD machine 

Attempt on implementing a secd virtual machine without any knowledge in virtual machines.
The machine is adapted from original paper [The Mechanical Evaluation of Expressions](https://academic.oup.com/comjnl/article/6/4/308/375725)
with slight changes.

## Instructions

### Building

The project uses nothing but c++ standard library and is built with cmake. So, simple 
```console
$ mkdir build
$ cmake -B build
$ make -C build
```
will be enough.

### Usage

There are two targets: `secd` - the machine itself, and `asm` - the assembler to get machine code to execute.
For the test, a factorial program is [present](./fact.asm) in the repo, it calculates the factorial of 7 recursively.

One can test it with
```console
$ ./build/asm fact.asm fact.func
$ ./build/secd fact.func
```

## Examples

There will be some samples of code to compile and test on the secd machine.

### Factorial

The factorial program is [present](./fact.asm) in the repo and duplicated in the [README](./README.md) [here](#factorial).

```asm
func:
    LDE x
    LDC 0
    EQ
    SEL then else
    STP

then:
    LDC 1
    STP

else:
    LDE f
    LDC 1
    LDE x
    SUB
    LDE f
    AP
    LDE x
    MUL
    STP

main:
    LDF func (f x)
    ST f
    LDE f
    LDE read
    AP
    LDE f
    AP
    LDE print
    AP
    STP
```

### `cons`, `car`, `cdr`

The machine does not have built-in list instructions, therefore they should be implemented manually

```asm
dispatch:
LDE c
LDC 0
EQ
SEL then else
STP

then:
LDE x
STP

else:
LDE y
STP

cons:
LDE x
LDE y
LDF dispatch // closure created
STP

car:
LDC 0
LDE l
AP
STP

cdr:
LDC 1
LDE l
AP
STP
```
