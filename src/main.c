#define _VM_IMPLEMENTATION
#include "virt_mach.h"

void print_usage_and_exit() {
    fprintf(stderr, "Usage: ./virtmach -action <anc|run> [-stack-size <size>] [-program-capacity <size>] [-limit <n>] <input> [output]\n");
    fprintf(stderr, "  -action <anc|run>         Action to perform ('anc' to assemble, 'run' to execute)\n");
    fprintf(stderr, "  -stack-size <size>        Optional stack size (default: 1024)\n");
    fprintf(stderr, "  -program-capacity <size>  Optional program capacity (default: 1024)\n");
    fprintf(stderr, "  -limit <n>                Optional instruction limit, -1 for no limit (default: -1)\n");
    fprintf(stderr, "  <input>                   Input file (for 'anc' or 'run')\n");
    fprintf(stderr, "  [output]                  Output file (only for 'anc' action)\n");
    exit(EXIT_FAILURE);
}


__int64_t parse_non_negative_int(const char *str) {
    __int64_t value = atoll(str);
    if (value < 0) {
        fprintf(stderr, "ERROR: Value must be non-negative.\n");
        print_usage_and_exit();
    }
    return value;
}

int main(int argc, char **argv) {
    __int64_t limit = -1;  // Default instruction limit: unlimited (-1)
    if (argc < 3) {
        print_usage_and_exit();
    }

    const char *action = NULL;
    const char *input = NULL;
    const char *output = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-action") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: Missing value for -action.\n");
                print_usage_and_exit();
            }
            action = argv[++i];
        } else if (strcmp(argv[i], "-stack-size") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: Missing value for -stack-size.\n");
                print_usage_and_exit();
            }
            vm_stack_capacity = parse_non_negative_int(argv[++i]);
        } else if (strcmp(argv[i], "-program-capacity") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: Missing value for -program-capacity.\n");
                print_usage_and_exit();
            }
            vm_program_capacity = parse_non_negative_int(argv[++i]);
        } else if (strcmp(argv[i], "-limit") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: Missing value for -limit.\n");
                print_usage_and_exit();
            }
            limit = atoll(argv[++i]);
        } else if (!input) {
            input = argv[i];  
        } else if (!output) {
            output = argv[i];  
        } else {
            fprintf(stderr, "ERROR: Too many arguments.\n");
            print_usage_and_exit();
        }
    }


    if (!action) {
        fprintf(stderr, "ERROR: Missing -action option.\n");
        print_usage_and_exit();
    }

    if (strcmp(action, "anc") == 0) {
        if (!input || !output) {
            fprintf(stderr, "./virtmach -action anc <input.vasm> <output.vm>\n");
            fprintf(stderr, "ERROR: expected input and output files for the 'anc' action.\n");
            exit(EXIT_FAILURE);
        }

        String_View source = slurp_file(input);
        Inst program[vm_program_capacity];
        size_t program_size = vm_translate_source(source, program, vm_program_capacity);
        vm_save_program_to_file(program, program_size, output);

        return EXIT_SUCCESS;

    } else if (strcmp(action, "run") == 0) {
        if (!input) {
            fprintf(stderr, "./virtmach -action run <file.vm>\n");
            fprintf(stderr, "ERROR: expected a .vm file for the 'run' action.\n");
            exit(EXIT_FAILURE);
        }

        VirtualMachine vm;
        vm_init(&vm);
        vm.program_size = vm_load_program_from_file(vm.program, input);
        vm_exec_program(&vm, limit);

        return EXIT_SUCCESS;

    } else {
        fprintf(stderr, "ERROR: Unknown action '%s'. Expected 'anc' or 'run'.\n", action);
        print_usage_and_exit();
    }

    return EXIT_FAILURE;
}