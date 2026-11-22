// nob.c - build script for pda
//
// bootstrap once:
//   cc -o nob nob.c
// then:
//   ./nob            build main
//   ./nob run        build and run main
//   ./nob test       build and run the test suite
//   ./nob format     clang-format all sources
//   ./nob clean      remove build outputs
//
// nob rebuilds itself whenever nob.c or thirdparty/nob.h changes.

#define NOB_IMPLEMENTATION
#include "thirdparty/nob.h"

#define MAIN_BIN "main"
#define TEST_BIN "tests/test"

#define BREW_LLVM "/opt/homebrew/opt/llvm"

static const char *main_inputs[] = {
    "src/main.c",
    "thirdparty/base58.c",
    "src/pda.c",
};
static const char *main_deps[] = {
    "src/main.c", "thirdparty/base58.c", "thirdparty/libbase58.h", "src/pda.c", "src/pda.h",
};

static const char *test_inputs[] = {
    "tests/test.c",
    "src/pda.c",
};
static const char *test_deps[] = {
    "tests/test.c",
    "src/pda.c",
    "src/pda.h",
};

static const char *format_files[] = {
    "src/fe.c",   "src/main.c",          "src/pda.c",
    "src/pda.h",  "tests/test.c",        "thirdparty/base58.c",
    "thirdparty/libbase58.h",
};

static void cc_common(Nob_Cmd *cmd) {
    nob_cmd_append(cmd, "clang", "-Wall", "-Wextra", "-Wpedantic");
#ifdef __APPLE__
    nob_cmd_append(cmd, "-I/opt/homebrew/opt/openssl@3/include",
                   "-L/opt/homebrew/opt/openssl@3/lib");
#endif
}

static bool build_bin(const char *out, const char **inputs, size_t inputs_count,
                      const char **deps, size_t deps_count) {
    int rebuild = nob_needs_rebuild(out, deps, deps_count);
    if (rebuild < 0) return false;
    if (rebuild == 0) {
        nob_log(NOB_INFO, "%s is up to date", out);
        return true;
    }

    Nob_Cmd cmd = {0};
    cc_common(&cmd);
    for (size_t i = 0; i < inputs_count; i++) nob_cmd_append(&cmd, inputs[i]);
    nob_cmd_append(&cmd, "-o", out, "-lcrypto");
    return nob_cmd_run(&cmd);
}

static bool build_main(void) {
    return build_bin(MAIN_BIN, main_inputs, NOB_ARRAY_LEN(main_inputs), main_deps,
                     NOB_ARRAY_LEN(main_deps));
}

static bool build_test(void) {
    return build_bin(TEST_BIN, test_inputs, NOB_ARRAY_LEN(test_inputs), test_deps,
                     NOB_ARRAY_LEN(test_deps));
}

static bool run_bin(const char *path) {
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, nob_temp_sprintf("./%s", path));
    return nob_cmd_run(&cmd);
}

static bool format(void) {
    const char *clang_format = getenv("CLANG_FORMAT");
    if (!clang_format) {
        clang_format = nob_file_exists(BREW_LLVM "/bin/clang-format") == 1
                           ? BREW_LLVM "/bin/clang-format"
                           : "clang-format";
    }

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, clang_format, "-i");
    for (size_t i = 0; i < NOB_ARRAY_LEN(format_files); i++) nob_cmd_append(&cmd, format_files[i]);
    return nob_cmd_run(&cmd);
}

static bool clean(void) {
    bool ok = true;
    if (nob_file_exists(MAIN_BIN) == 1) ok &= nob_delete_file(MAIN_BIN);
    if (nob_file_exists(TEST_BIN) == 1) ok &= nob_delete_file(TEST_BIN);
    return ok;
}

static void usage(const char *program) {
    nob_log(NOB_INFO, "usage: %s [build|run|test|format|clean]", program);
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "thirdparty/nob.h");

    const char *program = nob_shift(argv, argc);
    const char *sub     = argc > 0 ? nob_shift(argv, argc) : "build";

    if (strcmp(sub, "build") == 0) return build_main() ? 0 : 1;
    if (strcmp(sub, "run") == 0) return build_main() && run_bin(MAIN_BIN) ? 0 : 1;
    if (strcmp(sub, "test") == 0) return build_test() && run_bin(TEST_BIN) ? 0 : 1;
    if (strcmp(sub, "format") == 0) return format() ? 0 : 1;
    if (strcmp(sub, "clean") == 0) return clean() ? 0 : 1;

    nob_log(NOB_ERROR, "unknown subcommand: %s", sub);
    usage(program);
    return 1;
}
