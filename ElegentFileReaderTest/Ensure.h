#pragma once

namespace Saturn {

void __vectorcall RaiseCheckError(const char*);
void __vectorcall RaiseEnsureError(const char*);

} // namespace Saturn

//There is NO semicolon after the function calls immediately below.
//This is required for the macros to generate correct C++ in all cases.
//This is the same as the Google Test approach.

#define CHECK_FAILURE(x) Saturn::RaiseCheckError(x)
#ifdef NDEBUG
#define CHECK_WITH_3_PARAMETERS(x, op, y) ((x) op (y))
#else
#define CHECK_WITH_3_PARAMETERS(x, op, y) if ((x) op (y)); else CHECK_FAILURE(#x " " #op " " #y)
#endif
#define CHECK_TRUE(x) CHECK_WITH_3_PARAMETERS(x, ==, true)
#define CHECK_FALSE(x) CHECK_WITH_3_PARAMETERS(x, ==, false)
// cast to bool before calling CHECK_TRUE
#define CHECK_WITH_1_PARAMETER(x) CHECK_TRUE(bool(x))


#define ENSURE_FAILURE(expression_text) Saturn::RaiseEnsureError(expression_text)
#define ENSURE_WITH_3_PARAMETERS(x, op, y) if ((x) op (y)); else ENSURE_FAILURE(#x " " #op " " #y)
#define ENSURE_TRUE(x) ENSURE_WITH_3_PARAMETERS(x, ==, true)
#define ENSURE_FALSE(x) ENSURE_WITH_3_PARAMETERS(x, ==, false)
// cast to bool before calling ENSURE_TRUE
#define ENSURE_WITH_1_PARAMETER(x) ENSURE_TRUE(bool(x))

// CHECK.. and ENSURE.. with NO or 2 parameters are not defined - should give a meaningful compile error

#define CHECK_NO_ARG_EXPANDER() ,,,CHECK_WITH_NO_PARAMETERS
#define ENSURE_NO_ARG_EXPANDER() ,,,ENSURE_WITH_NO_PARAMETERS

#define FUNC_CHOOSER(_f1, _f2, _f3, _f4, ...) _f4
#define FUNC_RECOMPOSER(argsWithParentheses) FUNC_CHOOSER argsWithParentheses

// I've CamelCased the Macro parametr below becasue it is different: it is the partial macro name that is combined with a partial token to make another Macro name
#define CHOOSE_FROM_ARG_COUNT(Macro, ...) FUNC_RECOMPOSER((__VA_ARGS__, Macro##_WITH_3_PARAMETERS, Macro##_WITH_2_PARAMETERS, Macro##_WITH_1_PARAMETER, ))
#define MACRO_CHOOSER(Macro, ...) CHOOSE_FROM_ARG_COUNT(Macro, Macro##_NO_ARG_EXPANDER __VA_ARGS__ ())
#define CHECK(...) MACRO_CHOOSER(CHECK, __VA_ARGS__)(__VA_ARGS__)
#define ENSURE(...) MACRO_CHOOSER(ENSURE, __VA_ARGS__)(__VA_ARGS__)
