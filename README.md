# Table of Contents
- [Table of Contents](#table-of-contents)
- [Serio](#serio)
- [Installation](#installation)
- [Usage](#usage)
  - [API](#api)
    - [STL Types](#stl-types)
    - [Custom Types](#custom-types)
    - [Custom Containers](#custom-containers)
    - [Advances Usages](#advanced-usages)
  - [Supported Types](#supported-types)
  - [Debugging](#debugging)
  - [Notes](#notes)
- [Use Cases](#use-cases)
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
The main API consists of these functions:

```serialize``` -> takes an unlimited number of arguements (serializable data types) and serializes them into a byte array and returns the result.

```deserialize``` -> takes the byte array or char sequence and an unlimited number of arguements and deserializes the data into the arguements and returns the size of bytes consumed from byte array or char sequence.

```read``` -> reads binary data (the serialized data) from file into a byte array and returns weather it succeeds or not.

```write``` -> writes a byte array into a file and returns weather it succeeds or not.

```save``` -> does the job of ```serialize``` and ```write``` together. serializes arguements and writes them into a file and returns weather it succeeds or not.

```write``` -> does the job of ```read``` and ```deserialize``` together. reads data from file and deserializes it into arguements and returns weather it succeeds or not. 

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
With a little bit of work you can use this library to serialize your custom containers. For example:

``` c++
struct Custom : vector<int>
{
    using vector<int>::vector;
    
    template <typename Derived>
    inline void _serialize(Serio::SerializerOps<Derived>* C) const
    {
        auto& serializer = *static_cast<Derived*>(C);
        serializer << size();
        for (const auto& value : *this) serializer << value;
    }
    template <typename Derived>
    inline void _deserialize(Serio::DeserializerOps<Derived>* C)
    {
        auto& deserializer = *static_cast<Derived*>(C);
        size_type size;
        deserializer >> size;
        this->resize(size);
        for (size_type i = 0; i < size; ++i) deserializer >> at(i);
    }
};
```

Explanation:

Here we do the job of SERIO_REGISTER_CLASS manually. We define ```_serialize``` function in your class that serializes size first and then all the items. Then we define ```_deserialize``` function that deserializes size and resizes the container and then deserializes all the items.

Now your container is ready to be serialized and deserialized.

### Advances Usages
You can serialize conditional cases using this library! For example if you want to seralize a union of std::vector<int> and std::string you can do the following:
  
``` c++
struct Example
{
    std::variant<std::string, std::vector<int>> variant;

    template <typename Derived>
    inline void _serialize(Serio::SerializerOps<Derived>* C) const
    {
        auto& serializer = *static_cast<Derived*>(C);
        auto index = variant.index();
        serializer << index;

        if (index == 0)
            serializer << std::get<std::string>(variant);
        else if (index == 1)
            serializer << std::get<std::vector<int>>(variant);
    }
    template <typename Derived>
    inline void _deserialize(Serio::DeserializerOps<Derived>* C)
    {
        auto& deserializer = *static_cast<Derived*>(C);
        size_t index;
        deserializer >> index;

        if (index == 0)
        {
            variant = std::string();
            deserializer >> std::get<std::string>(variant);
        }
        else if (index == 1)
        {
            variant = std::vector<int>();
            deserializer >> std::get<std::vector<int>>(variant);
        }
    }
};
```

## Supported Types
Supported types: [C++ containers](http://www.cplusplus.com/reference/stl/), std::string, std::complex, std::pair, std::tuple, std::chrono::time_point, std::bitset, std::shared_ptr, std::unique_ptr, std::optional(C++17) and any structure that you register. If there are some types missing from this library you can report it on issues page.

## Debugging
If you get errors that look like these:

```
serio.h: error: no member named '_serialize' in 'class-name'
serio.h: error: no member named '_deserialize' in 'class-name'
```

This means that you are trying to serialize or deserialize a custom structure or class that you have not registed.

### Use Cases
This library can have a lot of applications. It can be used to process and save information on files and read them later. It can be used to turn text files into binary file and read it much faster later on. It can also be used to easily send and recieve data in binary form through sockets(network or otherwise).

## Notes
+ STL containers can be used interchangably when serializing the deserializing. For example you can serialize a ```std::vector``` and later deserialize it into a ```std::list```. This is both convinient and can speed up operations. For example you can serialize a ```std::vector``` and later deserialize it into a ```std::set``` to get unique items. With maps for example you can serialize a ```std::map<int, int>``` and later deserialize a ```std::vector<std::pair<int, int>>``` or vice versa.

+ If you want the serialized files to be completely machine-independent use fixed size integers instead of basic data types.

+ Doing ```using namespace Serio``` is not a good idea(because it may cause some conflicts).

+ When size of ```long double``` is 16 bytes, it is only supported where 128 integer is available (depends on compiler and hardware. see [this](https://gcc.gnu.org/onlinedocs/gcc-4.6.1/gcc/_005f_005fint128.html)). So be careful using it.

# Running the Tests
you won't have to run the tests but it you want to you have to install google test framework and compile ```tests.cpp``` file. For example with gcc you can do:

```
g++ tests.cpp -o tests -lgtest -O2
```

Then you can run the tests executable. Also note that tests might take a long time to compile so please be patient.

# Contribution and Questions
You can report bugs and ask questions on [issues page](../../issues). Also if you want to implement a feature you can open an issue and we can discuss it.

# License
This library is licensed under BSD 3-Clause license. You can read it [here](LICENSE.md).
