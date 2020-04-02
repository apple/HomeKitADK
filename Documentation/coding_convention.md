# Coding Conventions

## Code Style
Please use the following tools to auto-format your code before submitting a Pull Request.

## Linting source code
This project uses `clang-format` tool to lint and format the code.
```
./Tools/linters/clint.sh -h
```

## Linting shell scripts
```
./Tools/linters/shlint.sh
```

## Conventions

### Sample Application
- In DB.c, the following IIDs should be used:
  - 0x...0 Service IID.
  - 0x...1 Service Signature characteristic IID. Only present when the service has linked service or has properties.
  - 0x...2 Name characteristic IID. Services can have a Name characteristic if they are user visible.
  - 0x...3+ Regular characteristics, ordered the same way as in the service specification, with required characteristics
listed before optional characteristics.

When updating a DB file for a sample that has already been released, try to avoid changing existing IIDs.


### Error Handling
- Unexpected errors `HAPError` are used for exceptional circumstances such as communication failure with I2C,
or access problems with key-value store. Those errors should be modeled as `HAPError` and are in most cases
bubbled out to the caller until the next error handler.
- Expected errors are conditions such as a key that is not available in the key-value store, or a Wi-Fi configuration
that has not yet been set. Such errors should be modeled as separate out parameters, such as
`bool *found`, `bool *isSet`, `bool *isDefined`, `bool *isAvailable`, or `bool *isConfigured`. Do not use
`bool *isValid` to denote existence of a value.
- The idea is that a caller of a function in general should always be able to treat a `HAPError` as a boolean, and only
has to handle expected errors returned through the separate out parameters.
- When a HAPError is returned, the caller shall not make assumptions about the validity of any out-parameters.
- When kHAPError_None is returned, all applicable out parameters of the function must contain valid values.


### Specification References
- Always reference the spec if possible, and always use the exact same format to simplify "Find in Files" searches.
  - First line: `@see` or See followed by exact PDF Title as shown in title bar when opening it.
  - Second line: Section + Section number + exact Section Title.
  - Tables may be referenced as well, in that case use Table instead of Section.
  - Section references are preferred as tables may be harder to find.
- For code comments, quote exactly what part of the spec is referenced.
- Always use the most recent specification version where the feature is documented. If a feature becomes obsolete, use
the Doxygen `@obsolete` tag to specify the first revision it got removed. Avoid actually deleting enum values etc,
use `@obsolete` instead.

#### Example for documentation block
```
/**
 * ...
 *
 * @see HomeKit Accessory Protocol Specification R15
 *      Section 1.2.3.4 Some Sample Section
 */
```

#### Example for code comment
```
// ...
// See HomeKit Accessory Protocol Specification R15
// Section 1.2.3.4 Some Sample Section
```


### Definitions
- Definition name: kHAPCategory_PascalCase
- Always explicitly specify the type of the definition, e.g., `uint8_t`, `size_t`.
- Consider using enumerations instead, where possible.

```
/**
 * Brief description
 */
#define kHAPCategory_PascalCase ((size_t) 0x42)
```


### Enumerations
- Enumerations are never used to store out-of-range values. Instead, the underlying type is used, e.g., `uint8_t`.
Before casting to an enum, perform proper input validation. When receiving an enum, no thorough input validation is
necessary as it is already done.
- Type name: `HAPPascalCase`
- Case name: `kHAPTypeName_PascalCase` Always prefix with lower case `k`.
- If there is a default, uninitialized, 0 case, list it first, but do not set explicitly to `0`.
- If there is no default case, set first value to 1 to avoid accidents with `memset 0`. Do not set values to other cases.
- For flags, set the initial case to `1 << 0`, the next to `1 << 1` etc.
- Align documentation on column 37 if possible, otherwise use a full separate Doxygen style comment block. If there are
some long and some short cases, consider using the full comment block for all cases. A single-line Doxygen style
comment block may be used to condense information. If the enum is defined in an indented scope, columns advance by `4`
per indentation.
- In interfaces, always use `HAP_ENUM_BEGIN` / `HAP_ENUM_END` for portability across different compiler settings.

```
/**
 * Enum type documentation.
 */
HAP_ENUM_BEGIN(uint8_t, HAPEnumType) {
    kHAPEnumType_PascalCase1,       /**< Case 1 documentation. */
    kHAPEnumType_PascalCase2,       /**< Case 2 documentation. */

    /**
     * Case 3 documentation.
     *
     * - Remark 1.
     *
     * - Remark 2.
     */
    kHAPEnumType_ReallyReallyLongCase,

    /** Case 4 documentation. */
    kHAPEnumType_ReallyReallyLongCase2
} HAP_ENUM_END(uint8_t, HAPEnumType);
```


### Functions
- Function names: *HAPPascalCase*<br>
Use concise names and avoid abbreviations. Length does not matter here. If targeting an object, first mention the
object, e.g., `HAPAccessoryServerStartBridge` targets a `HAPAccessoryServer` object. In that case, also make sure that
the target object is the very first parameter.
- Parameter names: *camelCase*<br>
Maximum parameter length name is 20. Otherwise it will break the table in the documentation.
Use `HAP_UNUSED`, if the argument is not used in the function body. Do not cast to (void) for compiler warnings.
Pointers are assumed to always be non-NULL. Use `_Nullable` on pointer arguments that may be NULL.
- Parameter types use fixed size C99 integers, e.g., `uint8_t`, `int16_t`, `size_t`. Avoid `int`, `short`,
`unsigned char`. For booleans, use `bool`. Yes, it's just a typedef to int in C, but it improves readability.
- Return type must be `HAPError` if the function is expected to fail. If the function returns a value (non-void),
it must be marked with `HAP_RESULT_USE_CHECK`. If the function cannot fail and does not provide any outputs,
return void. If unsure whether a function may return an error, start with void return first. Errors that arise from
API misuse (invalid arguments, invalid state) crash / assert instead of returning error.
- Every parameter and return value is documented.
   - If a parameter is an out-parameter, [out] is attached after the `@param`.
   - If a parameter is an in/out parameter, [in, out] is attached after the `@param`.
   - `true` / `false` return values are written in lowercase in documentation.
   - `HAPError` return values are sorted in documentation in the same way as listed in the `HAPError` enum.
- GCC attributes one per line, sorted alphabetically. Apply GCC attributes to both declaration and definition of the
function.
- Function bodies are streamlined
  - Precondition checks: One precondition per line, checked in the order they appear in the function signature.
Casts from Refs to internal types should be made as close as possible to where their preconditions are checked.
Early returns may also be included as part of the precondition checks block if it makes sense.
  - HAPError err;
  - Function body.
- Use early returns if possible.
If returning because of an error, use a `HAPLog` statement to print the reason of the error before throwing. When
just re-throwing an error, do not log. The error is already logged when it was thrown. If a post-condition must be
checked even in error cases, wrap the function body in a second function that uses early returns, and check the
post-condition in the wrapping function.

```
/**
 * Brief documentation about the function.
 *
 * - Remark 1 about special considerations.
 *
 * - Remark 2 about special considerations.
 *
 * @param      arg1                 Argument 1 documentation.
 * @param      arg2                 Argument 2 documentation.
 * @param[out] bytes                Buffer that will be filled with data.
 * @param      maxBytes             Maximum number of bytes that may be filled into the buffer.
 * @param[in,out] numBytes          Effective number of bytes written to the buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer was not large enough.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPDoSomething(
    size_t arg1,
    bool arg2 HAP_UNUSED,
    void *bytes,      // _Nullable if bytes are _Nullable
    size_t maxBytes,
    size_t *numBytes) // _Nullable if bytes are _Nullable
{
    HAPPrecondition(arg1 > 42);
    HAPPrecondition(bytes);

    HAPError err;

    // ...

    return kHAPError_None;
}
```

### Header Files
- Name: `HAPPascalCase.h`. The file name does not have to be a prefix of all functions.
- If several items form a certain subgroup of a header, consider using a separate header file.
Name: `HAPBaseHeader+CategoryName.h`
- Include header files from `HAP.h`. Include category headers from their corresponding base header.
- Header guards: `HAP_UNDERSCORED_FILE_NAME_H`. Do not forget to update the header guard after file renames. Do not
forget to update the log category / subsystem in source files that implement the header after file renames.
- Always include `extern "C"` declaration, even in internal headers. Headers may move around over time and become
visible to C++. Also include the declaration when it is not necessary, for consistency. It doesn't hurt.
- Header files are streamlined:
   -  Copyright notice. Make sure it is EXACTLY the same in all files, so that automated re-copyrighting scripts work.
   -  Empty line.
   -  Header guard.
   -  Empty line.
   -  extern "C" declaration.
   -  `#include <system_header.h>`, sorted alphabetically. If some are in a subdirectory, place them in the end.
      System headers must only be included in HAPBase.h and in platform-specific code!
   -  Empty line, if system headers were included.
   -  `#include "platform_header.h"`, sorted alphabetically. This is for headers coming from external dependencies.
   -  Empty line, if platform headers were included.
   -  `#include "HAP.h"` / `#include "HAPPlatform.h"`, and categories of own header files, sorted alphabetically.
   -  Empty line.
   -  Enter `assume_nonnull` block.
   -  Header file contents.
   -  Exit `assume_nonnull` block.
   -  Empty line.
   -  If necessary, further `#include` statements, followed by empty line.
   -  Complete extern "C" declaration.
   -  Empty line.
   -  Complete header guard.
   -  Empty line.

```
#ifndef HAP_HEADER_FILE_NAME_H
#define HAP_HEADER_FILE_NAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <system_header.h>
#include "platform_header.h"
#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

// Header file contents.

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
```

### Switch Statements
- When switching over a closed enum, do not add a default case. Try to return from every case, and place a fatal error
after the switch. This ensures compiler warnings when new enum cases are added but are not explicitly handled.
- Use a separate scope for every case. (Empty cases are allowed to be condensed together, e.g., case 'c' / 'd' below).
- After the closing bracket of each case's scope, put either a `break;` statement or a `// Fallthrough.` comment.
- When switching on enumerations, add a default case like `default: HAPFatalError();` or `default: false`
as the final line to cover unexpected values.

```
switch (foo) {
    case 'a': {
        // Handle case.
    } break;
    case 'b': {
        // Handle case.
    } break;
    case 'c':
    case 'd': {
        // Handle cases.
    } // Fallthrough.
    case 'e': {
        // Handle case.
    } break;
}
```

- Exceptions are simple switch statements with early returns where putting an extra break statement after the case's
scope does not help with readability. If there are a lot of cases and the switch is just a conversion table from
one constant to another it can also be considered to remove the brackets to avoid spanning the switch across pages.

```
static const char *GetCurrentHeatingCoolingStateDescription(
    HAPCharacteristicValue_CurrentHeatingCoolingState state)
{
    switch (state) {
        case kHAPCharacteristicValue_CurrentHeatingCoolingState_Off:
            return "Off.";
        case kHAPCharacteristicValue_CurrentHeatingCoolingState_Heat:
            return "Heat. The Heater is currently on.";
        case kHAPCharacteristicValue_CurrentHeatingCoolingState_Cool:
            return "Cool. Cooler is currently on.";
        default: HAPFatalError();
    }
}
```

### Goto Statements
- In general, goto statements should be avoided. Especially, goto statements must not be used to jump backwards.
However, in the following scenarios, goto statements may be used.
- If a postcondition needs to be checked or final cleanup needs to be performed before returning from a function,
instead of using an early return, a goto statement may be used to handle that finalization.
- Goto statements may be used to break out of nested loops.
- Goto statements may also be used to break from a case statement of a switch on an enumeration.
This allows placing a `HAPFatalError()` immediately after the switch without providing a default case.
Not providing a default case allows the compiler to emit warnings if an enumeration value is not handled.
Placing a `HAPFatalError()` and goto label immediately after the switch allows catching unexpected value.
- Goto statements may be used when multiple cases of a switch prepare some variables but otherwise share their
implementation. In this case the goto label shall be placed immediately after the cases sharing the code,
before the definition of the next case label.
- Goto labels should use camelCase.

### Magic numbers
- Number literals are avoided in favor of named constants, except where a literal is more transparent, e.g
  - obvious offsets like `+/- 1`
  - offsets or shift distances and masks for bit manipulation like this:  `mantissa = (x >> 9) & 0x1FFFF;`
  - POSIX success `(0)` and most important error `(-1)` numbers
- Test code is allowed to contain magic numbers

### Dynamic memory allocation
- There are only a few places in the PAL where we do dynamic memory allocation, none in the HAP Library.
- Use the macro `HAPPlatformFreeSafe` to deallocate memory in cases where memory is allocated dynamically with libc.
- Use similar macros for third-party library deallocation.

### Use of parameters
- All public functions are documented with Doxygen comments.
- All const pointers and non-pointer values are inputs (caller provides data).
- All non-const pointers are outputs or inouts.
- In Doxygen, parameters where the caller must supply input data are not annotated.
- In Doxygen, parameters where the caller should use output data are annotated with [out].
- In Doxygen, parameters where the caller must supply input data and should use output data are annotated with [in,out],
except for "this" pointers which are the first parameters of functions that work with object-like data types.

### Use of void parameters
- We normally use typed parameters.
- If byte buffers are returned, void is used instead, to not force clients to use type casts. This is the same style as
Apple's mDNSResponder.


### Opaque structures
- Internal structures that are exposed publicly should be hidden by following the opaque pattern.
- Public header: Use the `HAP_OPAQUE` define with the first arguments defining the size in multiples of 8.
The associated data is aligned on 8 bytes. Name ends in `Ref`.

```
typedef HAP_OPAQUE(24) HAPFooRef;
```

- Private header: Specify real structure without "Ref" in name and static assert that it fits into a Ref.
  `typedef struct { ... } HAPFoo;`

```
HAP_STATIC_ASSERT(sizeof (HAPFooRef) >= sizeof (HAPFoo), HAPFoo);
```

- Functions: Always pass around refs. If internal fields need to be accessed, create second variable with _ postfix.

```
    HAPFooRef *foo_;
    HAPFoo *foo = (HAPFoo *) foo_;
```
