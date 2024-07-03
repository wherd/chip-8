#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define global static
#define internal static
#define persist static

typedef union opcode {
    uint16_t instruction;
    struct {
        uint8_t o : 4;
        uint8_t x : 4;
        uint8_t y : 4;
        uint8_t n : 4;
    };
    struct {
        uint8_t kk : 8;
    };
    struct {
        uint16_t nnn : 12;
    };
} opcode_t;

internal void print_help() {
    fprintf(stderr,
        "dasm8 - disassemble chip-8 rom\n\n"
        "usage: dasm8 [-o name] rom\n"
        "\n"
        "o : output filename\n"
    );
    exit(-1);
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        print_help();
    }

    const char *input = 0;
    const char *output = "out.ct8";

    for (int i=1; i < argc; ++i) {
        if (strncmp("-o", argv[i], 2) == 0) {
            if (++i >= argc) {
                print_help();
            }

            output = argv[i];
        } else if (input == 0) {
            input = argv[i];
        } else {
            print_help();
        }
    }

    if (input == 0) {
        print_help();
    }

    FILE *rom = fopen(input, "rb");
    if (!rom) {
        perror("error: ");
        return -1;
    }

    uint8_t data[4096] = {0};
    size_t len = fread(&data, sizeof(uint8_t), 4096, rom);
    fclose(rom);

    FILE *fp = fopen(output, "w+");
    if (!fp) {
        perror("error: ");
        return -1;
    }

    opcode_t opcode = {0};
    for (uint64_t i = 0; i < len; i += 2) {
        opcode.instruction = (data[i] << 8) | data[i+1];

        switch (opcode.o) {
            case 0x0: {
                switch (opcode.kk) {
                    case 0xE0: {
                        fprintf(fp, "CLS\n");
                    } break;
                    case 0xEE: {
                        fprintf(fp, "RET\n");
                    } break;
                    default: {
                        fprintf(fp, "SYS  0x%X\n", opcode.nnn);
                    } break;
                }
            } break;

            case 0x1: {
                fprintf(fp, "JP   0x%X\n", opcode.nnn);
            } break;

            case 0x2: {
                fprintf(fp, "CALL 0x%X\n", opcode.nnn);
            } break;

            case 0x3: {
                fprintf(fp, "SE   V%X, 0x%X\n", opcode.x, opcode.kk);
            } break;

            case 0x4: {
                fprintf(fp, "SNE  V%X, 0x%X\n", opcode.x, opcode.kk);
            } break;

            case 0x5: {
                if (opcode.n == 0) {
                    fprintf(fp, "SE   V%X, V%X\n", opcode.x, opcode.y);
                } else {
                    fprintf(fp, "TXT  0x%X\n", opcode.instruction);
                }
            } break;

            case 0x6: {
                fprintf(fp, "LD   V%X, 0x%X\n", opcode.x, opcode.kk);
            } break;

            case 0x7: {
                fprintf(fp, "ADD  V%X, 0x%X\n", opcode.x, opcode.kk);
            } break;

            case 0x8: {
                switch (opcode.n) {
                    case 0x0: {
                        fprintf(fp, "LD   V%X, V%X\n", opcode.x, opcode.y);
                    } break;
                    case 0x1: {
                        fprintf(fp, "OR   V%X, V%X\n", opcode.x, opcode.y);
                    } break;
                    case 0x2: {
                        fprintf(fp, "AND  V%X, V%X\n", opcode.x, opcode.y);
                    } break;
                    case 0x3: {
                        fprintf(fp, "XOR  V%X, V%X\n", opcode.x, opcode.y);
                    } break;
                    case 0x4: {
                        fprintf(fp, "ADD  V%X, V%X\n", opcode.x, opcode.y);
                    } break;
                    case 0x5: {
                        fprintf(fp, "SUB  V%X, V%X\n", opcode.x, opcode.y);
                    } break;
                    case 0x6: {
                        fprintf(fp, "SHR  V%X, V%X\n", opcode.x, opcode.y);
                    } break;
                    case 0x7: {
                        fprintf(fp, "SUBN V%X, V%X\n", opcode.x, opcode.y);
                    } break;
                    case 0xE: {
                        fprintf(fp, "SHL  V%X, V%X\n", opcode.x, opcode.y);
                    } break;
                    default: {
                        fprintf(fp, "TXT  0x%X\n", opcode.instruction);
                    } break;
                }
            } break;

            case 0x9: {
                if (opcode.n == 0) {
                    fprintf(fp, "SNE  V%X, V%X\n", opcode.x, opcode.y);
                } else {
                    fprintf(fp, "TXT  0x%X\n", opcode.instruction);
                }
            } break;

            case 0xA: {
                fprintf(fp, "LD   I, 0x%X\n", opcode.nnn);
            } break;

            case 0xB: {
                fprintf(fp, "JP   V0, 0x%X\n", opcode.nnn);
            } break;

            case 0xC: {
                fprintf(fp, "RND  V%X, 0x%X\n", opcode.x, opcode.kk);
            } break;

            case 0xD: {
                fprintf(fp, "DRW  V%X, V%X, 0x%X\n", opcode.x, opcode.y, opcode.n);
            } break;

            case 0xE: {
                switch (opcode.kk) {
                    case 0x9E: {
                        fprintf(fp, "SKP  V%X\n", opcode.x);
                    } break;
                    case 0xA1: {
                        fprintf(fp, "SKNP V%X\n", opcode.x);
                    } break;
                    default: {
                        fprintf(fp, "TXT  0x%X\n", opcode.instruction);
                    } break;
                }
            } break;

            case 0xF: {
                switch (opcode.kk) {
                    case 0x07: {
                        fprintf(fp, "LD   V%X, DT\n", opcode.x);
                    } break;
                    case 0x0A: {
                        fprintf(fp, "LD   V%X, K\n", opcode.x);
                    } break;
                    case 0x15: {
                        fprintf(fp, "LD   DT, V%X\n", opcode.x);
                    } break;
                    case 0x18: {
                        fprintf(fp, "LD   ST, V%X\n", opcode.x);
                    } break;
                    case 0x1E: {
                        fprintf(fp, "ADD  I, V%X\n", opcode.x);
                    } break;
                    case 0x29: {
                        fprintf(fp, "LD   F, V%X\n", opcode.x);
                    } break;
                    case 0x33: {
                        fprintf(fp, "LD   B, V%X\n", opcode.x);
                    } break;
                    case 0x55: {
                        fprintf(fp, "LD   [I], V%X\n", opcode.x);
                    } break;
                    case 0x65: {
                        fprintf(fp, "LD   V%X, [I]\n", opcode.x);
                    } break;
                    default: {
                        fprintf(fp, "TXT  0x%X\n", opcode.instruction);
                    } break;
                }
            } break;

            default: {
                fprintf(fp, "TXT  0x%X\n", opcode.instruction);
            } break;
        }

        *data += 2;
    }

    fclose(fp);
    return 0;
}
