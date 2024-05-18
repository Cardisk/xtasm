#include "dll.h"

#include <dlfcn.h>
#include <memory>

std::string compile(std::string handle, std::vector<std::unique_ptr<Instr>> &instructions) {
    // creating an handle for the dynamic library.
    void *dll_handle = dlopen(handle.c_str(), RTLD_LAZY);
    
    // crashing in case of error.
    if (!dll_handle) {
        auto msg = "Unable to open dynamic library '" + handle + "': ";
        msg += dlerror();
        msg += "\n";
        crash(msg);
    }

    // creating a link with the library entry point function.
    const char *(*dll_compile)(std::vector<std::unique_ptr<Instr>> &);

    // searching for the function.
    dll_compile = (const char *(*)(std::vector<std::unique_ptr<Instr>> &)) dlsym(dll_handle, "compile");

    // crashing in case of error.
    if (!dll_compile) {
        auto msg = "Unable to find symbol 'compile' inside ':" + handle + "': ";
        msg += dlerror();
        msg += "\n";
        crash(msg);
    }

    return std::string(dll_compile(instructions));
}
