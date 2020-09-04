# Table of Contents
- [Table of Contents](#table-of-contents)
- [Serio](#serio)
- [Build & Install](#build--install)
- [Usage](#usage)
- [Notes](#notes)
- [Supported Types](#supported-types)
- [Debugging](#debugging)
- [Use Cases](#use-cases)
- [Tests](#tests)
- [Contribution and Questions](#contribution-and-questions)
- [Contributing](#contributing)
- [License](#license)

# Serio 
Serio is a C++11 library (with support for new data types like variant and optional introduced in C++17) that makes serialization of C++ containers and other data types and custom classes and structures fast and easy. Features:

+ Fast
+ Extremely Easy to Use
+ Single-Header
+ Endian-Independent
+ No External Dependencies (Only STL)
+ Permissive License

# Build & Install
If you want to install the library you can clone the project by:

``` shell
git clone --recursive https://github.com/ShahriarSS/Serio.git
```

Then go into the project directory and create build directory:

``` shell
cd Serio
mkdir build
cd build
```

In order to build and install the library:

``` shell
cmake -DCMAKE_BUILD_TYPE=Release -DSERIO_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=/usr/local/ ..
cmake --build .
sudo cmake --install .
```

```sudo``` might be needed or not depending on the install destination. 

You can include the library like this:

``` c++
#include <serio/serio.h>
```

You can also add the project as CMake subdirectory and set the include path to ```${SERIO_INCLUDE_DIRS}```.

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
You won't have to run the tests but if you want to, do these in the project directory:

``` shell
mkdir build && cd build
cmake -DSERIO_BUILD_TESTS=ON ..
make && ./tests
```

Then you can run the tests executable. Also note that tests might take a long time to compile so please be patient.

# Contribution and Questions
You can report bugs, ask questions and request features on [issues page](../../issues).

# Contributing
You can report bugs, ask questions and request features on [issues page](../../issues). Pull requests are not accepted right now.

# License
This library is licensed under BSD 3-Clause permissive license. You can read it [here](LICENSE.md).
