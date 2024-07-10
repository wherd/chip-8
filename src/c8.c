#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>

#define global static
#define internal static
#define persist static

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE 10
#define MEMORY_SIZE 4096
#define PC_START 0x200
#define COLOR_ON 0xFFFFFFFF
#define COLOR_OFF 0xFF000000
#define FRAMES_PER_SECOND 60
#define MS_PER_CYCLE 1000 / FRAMES_PER_SECOND

typedef enum chipmode
{
    CHIP_8,
    CHIP_48
} chipmode_t;

typedef union opcode
{
    uint16_t instruction;
    struct
    {
        uint8_t n : 4;
        uint8_t y : 4;
        uint8_t x : 4;
        uint8_t o : 4;
    };
    struct
    {
        uint8_t kk : 8;
    };
    struct
    {
        uint16_t nnn : 12;
    };
} opcode_t;

typedef struct cpu
{
    opcode_t opcode;
    uint16_t pc;
    uint16_t i;
    uint8_t v[16];
    uint16_t stack[16];
    uint8_t sp;
    uint8_t draw;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t keys[16];
    uint8_t memory[MEMORY_SIZE];
    uint32_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];
    chipmode_t mode;
} cpu_t;

global uint8_t font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

global SDL_Scancode key_map[16] = {
    SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_A,
    SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C,
    SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V};

global uint64_t fastseed;

internal uint8_t fastrand(void)
{
    fastseed = 214013 * fastseed + 2531011;
    return (fastseed >> 8) & 0x7F;
}

internal void cpu_error(cpu_t *cpu)
{
    fprintf(stderr,
            "Error: OP: 0x%x (X: 0x%x, Y: 0x%x, N: 0x%x, KK: 0x%x, NNN: 0x%x) at location 0x%x could not be executed.\n",
            cpu->opcode.o,
            cpu->opcode.x,
            cpu->opcode.y,
            cpu->opcode.n,
            cpu->opcode.kk,
            cpu->opcode.nnn,
            cpu->pc);
}

internal int cpu_init(cpu_t *cpu, const char *filename)
{
    SDL_memset(cpu, 0, sizeof(cpu_t));
    SDL_memset4(cpu->screen, COLOR_OFF, sizeof(cpu->screen) / 4);
    SDL_memcpy(cpu->memory, font, sizeof(font));
    cpu->pc = PC_START;

    FILE *game = fopen(filename, "rb");
    if (!game)
    {
        return -1;
    }

    fread(&cpu->memory[PC_START], sizeof(uint8_t), MEMORY_SIZE - PC_START, game);
    fclose(game);

    fastseed = (uint64_t)&fastseed;
    return 0;
}

internal void cpu_fetch(cpu_t *cpu)
{
    cpu->opcode.instruction = cpu->memory[cpu->pc] << 8 | cpu->memory[cpu->pc + 1];
    cpu->pc += 2;
}

internal void cpu_update_timers(cpu_t *cpu)
{
    if (cpu->delay_timer > 0)
    {
        cpu->delay_timer--;
    }

    if (cpu->sound_timer > 0)
    {
        cpu->sound_timer--;
    }
}

internal void cpu_execute(cpu_t *cpu)
{
    switch (cpu->opcode.o)
    {
    case 0x0: {
        switch (cpu->opcode.kk)
        {
        case 0xE0: {
            SDL_memset4(cpu->screen, COLOR_OFF, sizeof(cpu->screen) / 4);
            cpu->draw = 1;
        } break;
        case 0xEE: {
            cpu->pc = cpu->stack[--cpu->sp];
        } break;
        default: {
            cpu_error(cpu);
        } break;
        }
    } break;
    case 0x1: {
        cpu->pc = cpu->opcode.nnn;
    } break;
    case 0x2: {
        cpu->stack[cpu->sp++] = cpu->pc;
        cpu->pc = cpu->opcode.nnn;
    } break;
    case 0x3: {
        if (cpu->v[cpu->opcode.x] == cpu->opcode.kk)
        {
            cpu->pc += 2;
        }
    } break;
    case 0x4: {
        if (cpu->v[cpu->opcode.x] != cpu->opcode.kk)
        {
            cpu->pc += 2;
        }
    } break;
    case 0x5: {
        if (cpu->v[cpu->opcode.x] == cpu->v[cpu->opcode.y])
        {
            cpu->pc += 2;
        }
    } break;
    case 0x6:
    {
        cpu->v[cpu->opcode.x] = cpu->opcode.kk;
    } break;
    case 0x7: {
        cpu->v[cpu->opcode.x] += cpu->opcode.kk;
    } break;
    case 0x8: {
        switch (cpu->opcode.n)
        {
        case 0x0: {
            cpu->v[cpu->opcode.x] = cpu->v[cpu->opcode.y];
        } break;
        case 0x1: {
            cpu->v[cpu->opcode.x] |= cpu->v[cpu->opcode.y];
            cpu->v[0xF] = 0;
        } break;
        case 0x2: {
            cpu->v[cpu->opcode.x] &= cpu->v[cpu->opcode.y];
            cpu->v[0xF] = 0;
        } break;
        case 0x3: {
            cpu->v[cpu->opcode.x] ^= cpu->v[cpu->opcode.y];
            cpu->v[0xF] = 0;
        } break;
        case 0x4: {
            uint8_t vf = cpu->v[cpu->opcode.y] > (0xFF - cpu->v[cpu->opcode.x]);
            cpu->v[cpu->opcode.x] += cpu->v[cpu->opcode.y];
            cpu->v[0xF] = vf;
        } break;
        case 0x5: {
            uint8_t vf = cpu->v[cpu->opcode.x] >= cpu->v[cpu->opcode.y];
            cpu->v[cpu->opcode.x] -= cpu->v[cpu->opcode.y];
            cpu->v[0xF] = vf;
        } break;
        case 0x6: {
            if (cpu->mode == CHIP_8) {
                cpu->v[cpu->opcode.x] = cpu->v[cpu->opcode.y];
            }
            uint8_t vf = cpu->v[cpu->opcode.x] & 1;
            cpu->v[cpu->opcode.x] /= 2;
            cpu->v[0xF] = vf;
        } break;
        case 0x7: {
            uint8_t vf = cpu->v[cpu->opcode.y] >= cpu->v[cpu->opcode.x];
            cpu->v[cpu->opcode.x] = (cpu->v[cpu->opcode.y] - cpu->v[cpu->opcode.x]);
            cpu->v[0xF] = vf;
        } break;
        case 0xE: {
            if (cpu->mode == CHIP_8) {
                cpu->v[cpu->opcode.x] = cpu->v[cpu->opcode.y];
            }
            uint8_t vf = (cpu->v[cpu->opcode.x] & 0x80) == 0x80;
            cpu->v[cpu->opcode.x] *= 2;
            cpu->v[0xF] = vf;
        } break;
        default: {
            cpu_error(cpu);
        } break;
        }
    } break;
    case 0x9: {
        switch (cpu->opcode.n)
        {
        case 0x0: {
            if (cpu->v[cpu->opcode.x] != cpu->v[cpu->opcode.y])
            {
                cpu->pc += 2;
            }
        } break;
        default: {
            cpu_error(cpu);
        } break;
        }
    } break;
    case 0xA: {
        cpu->i = cpu->opcode.nnn;
    } break;
    case 0xB: {
        switch (cpu->mode)
        {
        case CHIP_8: {
            cpu->pc = cpu->opcode.nnn + cpu->v[0];
        } break;
        case CHIP_48: {
            cpu->pc = cpu->opcode.nnn + cpu->v[cpu->opcode.x];
        } break;
        }
    } break;
    case 0xC: {
        cpu->v[cpu->opcode.x] = fastrand() & cpu->opcode.nnn;
    } break;
    case 0xD: {
        cpu->v[0xF] = 0;
        uint8_t wrap = cpu->v[cpu->opcode.x] >= SCREEN_WIDTH || cpu->v[cpu->opcode.y] >= SCREEN_HEIGHT;
        for (int y = 0; y < cpu->opcode.n; ++y)
        {
            for (int x = 0; x < 8; ++x)
            {
                uint8_t pixel = cpu->memory[cpu->i + y];
                if (pixel & (0x80 >> x))
                {
                    if (wrap == 1 || ((cpu->v[cpu->opcode.x] + x) < SCREEN_WIDTH && (cpu->v[cpu->opcode.y] + y) < SCREEN_HEIGHT))
                    {
                        int index = ((cpu->v[cpu->opcode.x] + x) % SCREEN_WIDTH) +
                                    ((cpu->v[cpu->opcode.y] + y) % SCREEN_HEIGHT) *
                                    SCREEN_WIDTH;

                        if (cpu->screen[index] == COLOR_ON)
                        {
                            cpu->v[0xF] = 1;
                            cpu->screen[index] = COLOR_OFF;
                        }
                        else
                        {
                            cpu->screen[index] = COLOR_ON;
                        }

                        cpu->draw = 1;
                    }
                }
            }
        }
    } break;
    case 0xE: {
        switch (cpu->opcode.kk)
        {
        case 0x9E: {
            if (SDL_GetKeyboardState(0)[key_map[cpu->v[cpu->opcode.x]]])
            {
                cpu->pc += 2;
            }
        } break;
        case 0xA1: {
            if (!SDL_GetKeyboardState(0)[key_map[cpu->v[cpu->opcode.x]]])
            {
                cpu->pc += 2;
            }
        } break;
        default: {
            cpu_error(cpu);
        } break;
        }
    } break;
    case 0xF: {
        switch (cpu->opcode.kk)
        {
        case 0x07: {
            cpu->v[cpu->opcode.x] = cpu->delay_timer;
        } break;
        case 0x0A: {
            cpu->pc -= 2;
            for (int i = 0; i < 16; ++i)
            {
                if (SDL_GetKeyboardState(0)[key_map[i]])
                {
                    cpu->v[cpu->opcode.x] = i;
                    cpu->pc += 2;
                    break;
                }
            }
        } break;
        case 0x15: {
            cpu->delay_timer = cpu->v[cpu->opcode.x];
        } break;
        case 0x18: {
            cpu->sound_timer = cpu->v[cpu->opcode.x];
        } break;
        case 0x1E: {
            cpu->i += cpu->v[cpu->opcode.x];
        } break;
        case 0x29: {
            cpu->i = cpu->v[cpu->opcode.x] * 5;
        } break;
        case 0x33: {
            cpu->memory[cpu->i] = cpu->v[cpu->opcode.x] / 100;
            cpu->memory[cpu->i + 1] = (cpu->v[cpu->opcode.x] / 10) % 10;
            cpu->memory[cpu->i + 2] = cpu->v[cpu->opcode.x] % 10;
        } break;
        case 0x55: {
            memcpy(cpu->memory + cpu->i, cpu->v, cpu->opcode.x + 1);
            switch (cpu->mode)
            {
            case CHIP_8: {
                cpu->i += cpu->opcode.x + 1;
            } break;
            case CHIP_48: {
                cpu->i += cpu->opcode.x;
            } break;
            }
        } break;
        case 0x65: {
            memcpy(cpu->v, cpu->memory + cpu->i, cpu->opcode.x + 1);
            switch (cpu->mode)
            {
            case CHIP_8: {
                cpu->i += cpu->opcode.x + 1;
            } break;
            case CHIP_48: {
                cpu->i += cpu->opcode.x;
            } break;
            }
        } break;
        default: {
            cpu_error(cpu);
        } break;
        }
    } break;
    default: {
        cpu_error(cpu);
    } break;
    }
}

internal void trc(int result_code, const char *msg)
{
    if (result_code)
    {
        fprintf(stderr, "%s\n", msg);
        exit(result_code);
    }
}

internal void trp(void *ptr, const char *msg)
{
    if (ptr == 0)
    {
        fprintf(stderr, "%s\n", msg);
        exit(-1);
    }
}

internal void print_help()
{
    fprintf(stderr,
            "c8, chip-8 emulator\n\n"
            "usage: c8 [-m mode] game\n\n"
            "  m : chip-8, chip-48 (default: chip-8)\n"
            "  h : show this message\n");
    exit(-1);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_help();
    }

    cpu_t cpu = {0};
    const char *filename = 0;

    for (int i = 1; i < argc; ++i)
    {
        if (strncmp("-m", argv[i], 2) == 0)
        {
            if (++i >= argc)
            {
                print_help();
            }

            if (strncmp("chip-8", argv[i], 6) == 0)
            {
                cpu.mode = CHIP_8;
            }
            else if (strncmp("chip-48", argv[i], 7) == 0)
            {
                cpu.mode = CHIP_8;
            }
            else
            {
                print_help();
            }
        }
        else if (strncmp("-h", argv[i], 2) == 0)
        {
            print_help();
        }
        else if (filename == 0)
        {
            filename = argv[i];
        }
        else
        {
            print_help();
        }
    }

    trc(cpu_init(&cpu, filename), "Unable to initialize emulator.");
    trc(SDL_Init(SDL_INIT_VIDEO), "Unable to initialize system.");

    SDL_Window *window = SDL_CreateWindow("Chip-8", SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, 0);
    trp(window, "Unable to initialize display.");

    SDL_Renderer *renderer = SDL_CreateRenderer(window, 0);
    trp(renderer, "Unable to initialize display.");

    trc(SDL_SetRenderLogicalPresentation(
        renderer,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_LOGICAL_PRESENTATION_LETTERBOX,
        SDL_SCALEMODE_NEAREST),
        "Unable to initialize display.");

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STATIC,
                                             SCREEN_WIDTH, SCREEN_HEIGHT);

    trp(renderer, "Unable to initialize display.");

    uint64_t last_time;
    uint8_t running = 1;

    while (running)
    {
        last_time = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT: {
                running = 0;
            } break;
            case SDL_EVENT_KEY_DOWN: {
                if (event.key.key == SDLK_ESCAPE)
                {
                    running = 0;
                }
            };
            }
        }

        for (int i = 0; i < 1000; ++i)
        { 
            cpu_fetch(&cpu);
            cpu_execute(&cpu);
            if (cpu.draw)
            {
                break;
            }
        }

        cpu_update_timers(&cpu);

        if (cpu.draw)
        {
            SDL_UpdateTexture(texture, 0, cpu.screen, SCREEN_WIDTH * sizeof(uint32_t));
            SDL_RenderTexture(renderer, texture, 0, 0);
            SDL_RenderPresent(renderer);

            cpu.draw = 0;
        }

        uint64_t time_elapsed = SDL_GetTicks() - last_time;
        if (time_elapsed < MS_PER_CYCLE)
        {
            SDL_Delay(MS_PER_CYCLE - time_elapsed);
        }
    }

    return 0;
}
