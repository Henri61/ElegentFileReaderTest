#include "pch.h"

#include <Windows.h>
#include <DbgHelp.h>
#include <intrin.h>

#if defined(_DEBUG) || defined(NDEBUG)
#include <exception>
#endif

namespace Saturn {
namespace {

__declspec(noinline) void __vectorcall RaiseError(const char* check_or_ensure, const char* condition, void* const p_return_address = ::_ReturnAddress())
{
    //Get a location within the "call" assembly instruction which corresponds to the line of code that led to the exception.
    void* const p_error_address = reinterpret_cast<void*>(reinterpret_cast<size_t>(p_return_address) - 1);

    //Can't Use wchar_t for unicode file names - exceptions will only accept non-wide characters.
    char message[2000];
    char* p_output = &message[0];

    HMODULE module_containing_return_address = nullptr;
    const unsigned __int32 module_found_status = ::GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        static_cast<char*>(p_error_address),
        &module_containing_return_address);

    p_output += ::wsprintfA(p_output, "%s Error - Does not satisfy: %s\r\n", check_or_ensure, condition);
    //Can't check - Error in error handler.
    ::GetModuleFileNameA(module_containing_return_address, p_output, MAX_PATH);
    p_output += ::strlen(p_output);

    const size_t RVA = (module_found_status == 0 ? 0
        : static_cast<char*>(p_error_address) - reinterpret_cast<char*>(module_containing_return_address));

    p_output += ::wsprintfA(p_output, "\r\nRVA: %p", RVA);

    const bool symbols_exist = (::SymInitialize(::GetCurrentProcess(), nullptr, TRUE) == TRUE);
    if (symbols_exist) {
        p_output += ::wsprintfA(p_output, "\r\n");

        //Can't check - Error in error handler.
        ::GetModuleFileNameA(module_containing_return_address, p_output, MAX_PATH);
        p_output += ::strlen(p_output);

        char temp_data[sizeof IMAGEHLP_SYMBOL + MAX_PATH]; //Allocate extra space to store the symbol name.
        IMAGEHLP_SYMBOL& image_help_symbol = *reinterpret_cast<IMAGEHLP_SYMBOL*>(temp_data);
        image_help_symbol.SizeOfStruct = sizeof image_help_symbol;
        image_help_symbol.Address = 0;
        image_help_symbol.Size = 0;
        image_help_symbol.Flags = 0;
        image_help_symbol.MaxNameLength = MAX_PATH;
        image_help_symbol.Name[1] = '\0';
        //Can't check - Error in error handler - frequent errors when no information is available for a given module.
        ULONG_PTR displacement_64 = 0;
        if (::SymGetSymFromAddr(::GetCurrentProcess(), reinterpret_cast<size_t>(p_error_address), &displacement_64, &image_help_symbol) == TRUE)
            p_output += ::wsprintfA(p_output, "!%s+0x%x", image_help_symbol.Name, static_cast<unsigned __int32>(displacement_64));

        IMAGEHLP_LINE image_help_line;
        image_help_line.SizeOfStruct = sizeof image_help_line;
        //Can't check - Error in error handler - frequent errors when no information is available for a given module.
        unsigned long displacement = static_cast<unsigned __int32>(displacement_64);
        if (::SymGetLineFromAddr(::GetCurrentProcess(), reinterpret_cast<size_t>(p_error_address), &displacement, &image_help_line) == TRUE)
            p_output += ::wsprintfA(p_output, "\r\n%s, line %u + %u bytes", image_help_line.FileName, image_help_line.LineNumber, displacement);

        //Can't check - Error in error handler.
        ::SymCleanup(::GetCurrentProcess());
    }

    throw ::std::exception(message);
}

} // end of anonymous namespace

// don't inline these 2 for the stack mechanism to work - according to Janagan
// (they don't seem to inline anyway - but let's be explicit)
__declspec(noinline) void __vectorcall RaiseCheckError(const char* condition)
{
    RaiseError("CHECK", condition);
}

__declspec(noinline) void __vectorcall RaiseEnsureError(const char* condition)
{
    RaiseError("ENSURE", condition);
}

} // namespace Saturn
