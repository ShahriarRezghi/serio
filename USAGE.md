# Table of contents
- [Table of contents](#table-of-contents)
- [Introduction](#introduction)
- [String API](#string-api)
- [Raw Buffer API](#raw-buffer-api)
- [Stream API](#stream-api)
- [STL Data Types](#stl-data-types)
- [Custom Structures and Classes](#custom-structures-and-classes)
- [Custom Containers](#custom-containers)
- [Raw Arrays](#raw-arrays)
- [Advanced Usage](#advanced-usage)

# Introduction
This library is easy to use. Here is an explanation of the API. ```Serio``` has two type of APIs:

+ Serialization to and deserialization from buffers which is preferred way of using ```Serio``` because this API is very fast.
+ Serialization to and deserialization from streams which is used where the memory is not enough to use the buffer API or you have a special stream that has to be used instead of buffer API.

Functions that serialize take unlimited number of supported data types and serialize them into a string or raw array or stream.

Functions that deserialize take unlimited number of supported data types and deserialize them from a string or raw array or stream.

# String API
You can serialize data types into a string of characters (```Serio::ByteArray``` which is std::basic_string<char>). Here is an example of serializing to string:

``` c++
int A = 0, B = 0;
Serio::ByteArray str = Serio::serialize(A, B);
```

You can deserialize data types from a string of characters. Here is an example of deserializing from string:

``` c++
int A, B;
Serio::ByteArray str = "<serialized-data>";
Serio::deserialize(str, A, B);
```

# Raw Buffer API
You can serialize data types into a char sequence. Here is an example of serializing to raw buffer:

``` c++
int A, B;
auto data = new char[Serio::size(A, B)];
Serio::fill(data, A, B);
```

You can set the buffer size large enough to not be concerned about overflow or you can know the exact size by using ```Serio::size()``` function.

You can deserialize data types from a char sequence. Here is an example of deserializing from raw buffer:

``` c++
int A, B;
char* data = "<serialized-data>";
Serio::deserialize(data, A, B);
```

# Stream API
You can serialize data types into a output stream of chars. Here is an example of serializing to stream:

``` c++
int A, B;
std::ostringstream stream;
Serio::streamSerialize(&stream, A, B);
```

You can deserialize data types from a input stream of chars. Here is an example of deserializing from stream:

``` c++
int A, B;
std::istringstream stream;  // contains serialized data
Serio::streamDeserialize(&stream, A, B);
```

In the previous examples we serialize and deserialize ints. Now we'll pay attention to the other data types

# STL Data Types
Supported STL types are [C++ containers](http://www.cplusplus.com/reference/stl/), std::string, std::complex, std::pair, std::tuple, std::chrono::time_point, std::bitset, std::shared_ptr, std::unique_ptr, std::optional(C++17), std::variant(C++17). Here is an example:

``` c++
std::string A = "Hi";
std::vector<int> B = {1, 2, 3};
std::list<std::vector<int>> C = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
std::pair<int, int> D = {1,2};
Serio::ByteArray str = Serio::serialize(A,B,C,D);
```
As you can see you can nest the supported data types and use this library on it. You can deserialize the above code like this:

``` c++
std::string A;
std::vector<int> B;
std::list<std::vector<int>> C;
std::pair<int, int> D;

Serio::ByteArray str = "<serialized-data>";
Serio::deserialize(str, A, B, C, D);
```

# Custom Structures and Classes
You can make Custom classes and structures usable with just one line of code using ```SERIO_REGISTER``` macro. For example:

``` c++
struct Point
{
    int x, y;
    SERIO_REGISTER(x, y)
};

std::list<Point> points = {{0, 0}, {1, 1}};
Serio::ByteArray str = Serio::serialize(points);
```

Here is an example deserialization of above example:

``` c++
std::list<Point> points;
Serio::ByteArray str = "<serialized-data>";
Serio::serialize(points);
```

# Custom Containers
Here is an example of how to make custom containers compatible. Since the size of container is variable we can't use ```SERIO_REGISTER``` macro like we did before. We'll have to implement some of the details ourselves.

``` c++
class Container
{
    std::vector<int> data;

private:
    template <typename Derived>
    friend class Serio::SerializerOps;
    template <typename Derived>
    friend class Serio::DeserializerOps;

    template <typename Serializer>
    inline void _serialize(Serializer& C) const
    {
        C << Serio::Size(data.size());
        for (const auto& value : data) C << value;
    }
    template <typename Deserializer>
    inline void _deserialize(Deserializer& C)
    {
        Serio::Size size;
        C >> size;
        data.resize(size_t(size));
        for (Serio::Size i = 0; i < size; ++i) C >> data[i];
    }
};
```

Here we do the job of ```SERIO_REGISTER``` manually. We define ```_serialize``` function in your class that serializes size first and then all the items. Then we define ```_deserialize``` function that deserializes size and resizes the container and then deserializes all the items. Please note that if you serialize size of vector it will have size_t type and it's size will be platform dependent so we use ```Serio::Size``` here so the serialized data will be portable. Now your container is ready to be serialized and deserialized.

# Raw Arrays

# Advanced Usage