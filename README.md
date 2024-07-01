# CHIP-8

Yet another chip-8 emulator.

## Building and Running:

To build the project run:
```
make
```

Running from the command line:
```
c8 [-m mode] game
```

### Dependencies:
GNU C compiler<br>
Command line tools if you are on Mac OS<br>
[SDL3](https://wiki.libsdl.org/SDL3/FrontPage) installed on your machine<br>
A copy of a Chip-8 Rom

## Keyboard Layout:

### Chip8 Keypad:
|   |   |   |   |
|---|---|---|---|
| 1 | 2 | 3 | C |
| 4 | 5 | 6 | D |
| 7 | 8 | 9 | E |
| A | 0 | B | F |

### Emulator Keyboard Mapping:
|   |   |   |   |
|---|---|---|---|
| 1 | 2 | 3 | 4 |
| Q | W | E | R |
| A | S | D | F |
| Z | X | C | V |

## TODO

- [ ] Sound
- [ ] Implement other Chip-8 variants
- [ ] Chip-8 compiler
- [ ] Chip-8 disassembler
- [ ] Chip-8 debugger

## CHIP-8 Variants

- [x] CHIP-8
- [ ] CHIP-8-D6800
- [ ] CHIP-8E
- [ ] CHIP-8I
- [ ] CHIP-8X
- [ ] CHIP-10
- [ ] CHIP-48
- [ ] MEGACHIP
- [ ] SCHIP-1.0
- [ ] SCHIP-1.1
- [ ] SCHIPC
- [ ] XO-CHIP
