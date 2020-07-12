# Table of Contents
- [Table of Contents](#table-of-contents)
- [Serio](#serio)
- [Installation](#installation)
- [Usage](#usage)
- [Notes](#notes)
- [Supported Types](#supported-types)
- [Debugging](#debugging)
- [Use Cases](#use-cases)
- [Tests](#tests)
- [Contribution and Questions](#contribution-and-questions)
- [TODO](#todo)
- [License](#license)

# Serio 
Serio is a C++11 library (with support for new data types like variant and optional introduced in C++17) that makes serialization of C++ containers and other data types and custom classes and structures possible. Features:

+ Fast
+ Extremely Easy to Use
+ Single-Header
+ Endian-Independent
+ No External Dependencies (Only STL) 
+ Permissive License

# Installation
If you plan to run the tests please clone the project recursively:

``` shell
git clone --recursive https://github.com/ShahriarSS/Serio.git
```

You can use CMake to install the library (recommended way). Do these in the project directory:

``` shell
mkdir build && cd build
cmake -DBUILD_TESTS=OFF .. && cmake --install .
```

If you install it this way you can include the library with:

``` c++
#include <serio/serio.h>
```

Or you can copy ```serio.h``` file into your project directory. If you want a system-wide solution you can copy ```serio.h``` to ```/usr/include``` or ```/usr/local/include``` on your Linux (the second location is recommended).

# Usage
You can checkout [USAGE.md](USAGE.md) to learn how to use the library.

# Notes
+ STL containers can be used interchangeably when serializing the deserializing. For example you can serialize a ```std::vector``` and later deserialize it into a ```std::list```. This is both convenient and can speed up operations. For example you can serialize a ```std::vector``` and later deserialize it into a ```std::set``` to get unique items. With maps for example you can serialize a ```std::map<int, int>``` and later deserialize a ```std::vector<std::pair<int, int>>``` or vice versa. Please note that ```std::vector<bool>``` is stored in a compact way and can't be deserialized as ```std::list<bool>``` or ```std::deque<bool>```.
+ If you want the serialized files to be completely machine-independent use fixed size integers instead of basic data types.
+ Doing ```using namespace Serio``` is not a good idea(because it may cause some conflicts).

# Supported Types
Supported data types: [C++ containers](http://www.cplusplus.com/reference/stl/), raw arrays, std::string, std::complex, std::pair, std::tuple, std::chrono::time_point, std::bitset, std::shared_ptr, std::unique_ptr, std::optional (C++17), std::variant (C++17) and any struct or class that you register. If there is a data type in STL that you want to be supported (and is not) please open an issue on the [issues page](../../issues).

# Debugging
Here is tip to help you debug a specific error using this library. If you get errors that look like these:

``` log
serio.h: error: no member named '_serialize' in '<class-name>'
serio.h: error: no member named '_deserialize' in '<class-name>'
```

This means that you are trying to serialize or deserialize a custom structure or class that you have not registered.

# Use Cases
This library can have a lot of applications. Here is some:

+ It can be used to save processed information on files and read them later a lot faster (e.g. saving processed datasets).
+ It can be used to turn text files into binary file and read it much faster later on.
+ The serialized can be used to easily send and receive data in binary form through sockets (network or otherwise).
+ It can also be used to save program options and restore them on startup.

# Tests
The tests are written with ```Google Test``` library. You won't have to run the tests but if you want to do these in the project directory:

``` shell
mkdir build && cd build
cmake -DBUILD_TESTS=ON ..
make && ./Tests
```

Then you can run the tests executable. Also note that tests might take a long time to compile so please be patient.

# Contribution and Questions
You can report bugs, ask questions and request features on [issues page](../../issues).

# TODO
You can view the TODO file [here](TODO.md) to see what is left to be done.

# License
This library is licensed under BSD 3-Clause permissive license. You can read it [here](LICENSE.md).
