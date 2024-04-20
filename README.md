# ⚙️  XTASM

XTASM is a pure educational project which tries to be more user-friendly than assembly.

The purpose of this project is to have a less complex and higher-level syntax mantaining
some of the low-level features that assembly has.

The last goal of the development is to have a small working language (obviously not that efficient) 
with which you can implement some basic stuff.

## Instruction Set

Arithmetics:
- ADD
- SUB

Data:
- VARIABLES (prefix '.')
- MOV

Data Wrappers:
- ENUM
- REGISTERS (prefix '$')

Branching:
- LABELS (prefix ':')
- JMP

Branching Wrappers:
- IF-ELSE

Syscalls Wrappers:
- EXIT

Loops Wrappers:
- WHILE
- FOR
- LOOP
- BREAK

## Development Goals

- [X] Simple syntax that doesn't go against the user
- [X] Good error reporting during lexing and parsing
- [ ] Native execution (x86 && arm)
- [ ] Both 32 and 64 bit
- [ ] Self hosted (last goal)
