# Table of Contents
- [Table of Contents](#table-of-contents)
- [Serio](#serio)
- [Installation](#installation)
- [Usage](#usage)
  - [API](#api)
    - [STL Types](#stl-types)
    - [Custom Types](#custom-types)
    - [Custom Containers](#custom-containers)
  - [Supported Types](#supported-types)
  - [Debugging](#debugging)
  - [Notes](#notes)
- [Running the Tests](#running-the-tests)
- [Contribution and Questions](#contribution-and-questions)
- [License](#license)

# Serio
Serio is a C++11 library that makes serialization of C++ containers and other data types and custom classes and structures possible. Features:
+ Fast
+ Easy to Use
+ Endian-Independent
+ Single-Header
+ No External Dependencies (Only STL) 

# Installation
You can just copy ```serio.h``` file into your project directory and you're done!
If you want a system-wide solution you can copy ```serio.h``` to ```/usr/include``` or ```/usr/local/include``` on your Linux (the second location is recommended).

# Usage
## API
The main API consists of two main functions and two helper functions. You can serialize combinations of STL and custom data types with ```serialize``` function and deserialize them with ```deseialize``` function.
```serialize``` function takes unlimited number of arguements and serializes them and returns an strings of bytes. 
You can then write the data to a file with ```write``` function. This function takes the path of the file and data and writes the data into the file and returns weather or not it succeeds. 
You can read the serialized data from file with ```read``` function. This function takes the path of the file and data and reads the contents into data and returns weather or not it succeeds. 
```deserialize``` function takes the data and the arguements and deserializes data into arguements.

### STL Types
You can serialize and deserialize STL container and other types (see [supported types](#supported-types) for more details). For example:

``` c++
std::vector<int> vector1 = {1, 2, 3, 4};
auto data = Serio::serialize(vector1);

std::vector<int> vector2;
Serio::deserialize(data, vector2);
```

Here ```vector1``` and ```vector2``` are equal.
Here is and example of writing data into a file and reading it:

``` c++
int i = 10;
double d = 12;
std::vector<int> vector1 = {1, 2, 3, 4};
auto data = Serio::serialize(vector1, i, d);
Serio::write("test-data.bin", data);

std::vector<int> vector2;
Serio::ByteArray newData;
Serio::read("test-data.bin", newData);
Serio::deserialize(newData, vector2, i, d);
```

You can use some types interchangably when serializing and deserializing. See [Notes](#notes) section for more details.

### Custom Types
You can make Custom classes and structs serializable with just one line of code. For example:

``` c++
struct Point
{
    int x, y;
    SERIO_REGISTER_CLASS(x, y)
};

std::list<Point> points;
points.push_back(Point{10, 20});
points.push_back(Point{20, 30});

auto data = Serio::serialize(points);
Serio::deserialize(data, points);
```
 
Also note that structures or classes must be default constructable.

### Custom Containers
With a little bit of work you can use this library to serialize your custom container. For example:

``` c++
struct Custom : vector<int>
{
    using vector<int>::vector;
};

namespace Serio
{
Calculator& operator<<(Calculator& S, const Custom& C)
{
    S << Size(C.size());
    for (const auto& V : C) S << V;
    return S;
}

Serializer& operator<<(Serializer& S, const Custom& C)
{
    S << Size(C.size());
    for (const auto& V : C) S << V;
    return S;
}

Deserializer& operator>>(Deserializer& S, Custom& C)
{
    C.clear();

    Size size;
    S >> size;

    for (Size i = 0; i < size; ++i)
    {
        int temp;
        S >> temp;
        C.push_back(temp);
    }

    return S;
}
};  // namespace Serio
```

Explanation:

You have to write three functions in the namespace. First one helps the library determine the size of data to be able to allocate data fast. Here we first pass the size of container to Size Calculator and then every item in the container. The second function serializes the data and works similar to first one. The Third one deserializes the data. Here we first clear the container and then read size and the elements.

Now your container is ready to be serialized and deserialized.

## Supported Types
Supported types: [C++ containers](http://www.cplusplus.com/reference/stl/), std::string, std::complex, std::pair, std::tuple, std::chrono::time_point, std::bitset, std::shared_ptr, std::unique_ptr, std::optional(C++17) and any structure that you register. If there are some types missing from this library you can report it on issues page.

## Debugging
If you get errors that look like these:

```
serio.h: error: no member named '_serialize' in 'class-name'
serio.h: error: no member named '_deserialize' in 'class-name'
```

This means that you are trying to serialize or deserialize a custom structure or class that you have not registed.

## Notes
STL containers can be used interchangably when serializing the deserializing. For example you can serialize a ```std::vector``` and later deserialize it into a ```std::list```. This is both convinient and can speed up operations. For example you can serialize a ```std::vector``` and later deserialize it into a ```std::set``` to get unique items. With maps for example you can serialize a ```std::map<int, int>``` and later deserialize a ```std::vector<std::pair<int, int>>``` or vice versa.

To be completely machine-independent use fixed size integers.

Doing ```using namespace Serio``` is not a good idea(because it may cause conflicts).

When size of ```long double``` is 16 bytes, it is only supported where 128 integer is available (depends on compiler and hardware. see [this](https://gcc.gnu.org/onlinedocs/gcc-4.6.1/gcc/_005f_005fint128.html)). So be careful using it.

# Running the Tests
you won't have to run the tests but it you want to you have to install google test framework and compile ```tests.cpp``` file. For example with gcc you can do:

```
g++ tests.cpp -o tests -lgtest -O2
```

Then you can run the tests executable. Also note that tests might take a long time to compile so please be patient.

# Contribution and Questions
You can report bugs and ask questions on [issues page](../../issues).

# License
This library is licensed under BSD 3-Clause license. You can read it [here](LICENSE.md).
