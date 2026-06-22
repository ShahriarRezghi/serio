# Serio

A fast, modern C++ serialization library. Turn C++ values into a compact byte stream — or human-readable JSON / XML — and read them back with a single call.

- **Three backends, one API** — Binary, JSON, or XML; switched with a single template argument
- **Batteries included** — nearly the entire STL supported out of the box, plus Qt and Boost adapters
- **Optional compression and encryption** — zstd and password-based encryption for the binary backend
- **Extensible** — register custom types with a single macro
- **Endian-independent** and **permissively licensed** under BSD 3-Clause

## Quick Start

```cpp
#include <serio/serio.h>

std::vector<int> numbers = {1, 2, 3};
std::string name = "serio";

std::string bytes = Serio::serialize<Serio::Binary>({}, numbers, name);
Serio::deserialize<Serio::Binary>({}, bytes, numbers, name);
```

## Documentation

Full documentation — installation, API reference, supported types, custom types, options, and more — is at **https://serio.readthedocs.io/en/latest/**.

## License

BSD 3-Clause — see [LICENSE.md](LICENSE.md).
