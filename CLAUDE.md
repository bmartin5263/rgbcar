## Style

- 2-spaces for indentation
- `PascalCase` class names, file names, type aliases of class/struct types, static methods
- `camelCase` method names and variable names
- `snake_case` type aliases of primitive types
- Split classes into header and implementation files when possible

## Language

- C++20
- No concepts (not available in compiler)
- Be as const-correct as possible
- Prefer trailing return types and braces on same line for functions `constexpr auto function() -> type {`

## Code Verification

- Compile with `/opt/homebrew/bin/pio run -e debug`

## Code Generation

- Use C++ standard library types over any implementation-defined types unless the implementation-defined offers necessary features
- Templates are found in `/templates`
  - Use them as reference when asked 
  - For example: When asked "Create logging subsystem" then `/templates/subsystem` will be your starting point for class and file structure
- `#include "SourceHeader.h"` for headers in this codebase, `#include <VendorHeader.h>` for external and system header files
- Comments should not state the obvious, or refer to historical fixes.
- Follow the Rule of 5
  - Meaning, if any 1 of the 5 compiler generated functions are overridden/defaulted, the rest should be declared

## Refactoring

- Avoid changing explicit class parameter types to `auto` for lambdas unless the type is primitive

## Testing

- use `constexpr` over `const`