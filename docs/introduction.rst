Introduction
============

**Serio** is a fast, modern C++ serialization library. It turns C++ values —
standard containers, common standard-library types, and your own structs and classes — into a
compact byte stream (or human-readable JSON / XML) and reads them back with a single call.

Highlights:

- **Three backends, one API.** The same code can emit **Binary**, **JSON**, or **XML**; the
  format is picked with a compile-time template argument.
- **Fast.** Arithmetic containers take a bulk-copy fast path, and the binary format is compact.
- **Endian-independent.** Data written on one machine reads back correctly on another.
- **Batteries included.** Almost the entire STL is supported out of the box, plus optional
  zstd compression and password-based encryption for the binary backend.
- **Extensible.** Register your own types with a single macro, or adapt third-party types
  without modifying them. Optional adapters for **Qt** and **Boost** containers ship with the
  library.
- **Permissively licensed** under BSD 3-Clause.

.. contents:: Table of Contents
   :local:
   :depth: 2

Installation
------------

Requirements
~~~~~~~~~~~~~

- A **C++11** compiler is the minimum. Some types only become available under newer standards
  (C++17, C++20, C++23) — these are marked in `Supported Types`_.
- **CMake ≥ 3.21**.
- The repository uses git submodules (zstd, libsodium, RapidJSON, RapidXML, GoogleTest). Clone
  recursively so they are present:

  .. code-block:: shell

     git clone --recursive https://github.com/ShahriarSS/Serio.git

  If you already cloned without ``--recursive``, run:

  .. code-block:: shell

     git submodule update --init --recursive

Build options
~~~~~~~~~~~~~

Serio is configured through standard CMake options:

.. list-table::
   :header-rows: 1
   :widths: 32 14 54

   * - Option
     - Default
     - Effect
   * - ``SERIO_ENABLE_COMPRESSION``
     - ``ON``
     - Build zstd support so binary output can be compressed.
   * - ``SERIO_ENABLE_ENCRYPTION``
     - ``ON``
     - Build libsodium support so binary output can be encrypted.
   * - ``SERIO_BUILD_SHARED``
     - ``ON``
     - Build a shared library (``OFF`` builds static).
   * - ``SERIO_BUILD_TESTS``
     - ``ON``
     - Build the test suite.
   * - ``SERIO_INSTALL_LIB``
     - master
     - Install rules (on by default for a top-level build).

.. note::

   **Header-only vs. compiled.** When **both** compression and encryption are disabled, Serio is
   a pure header-only (``INTERFACE``) library — nothing needs to be built. When either feature is
   enabled (the default), Serio compiles a small library that links zstd and/or libsodium. A
   static build is **not** compatible with encryption enabled. Either way the JSON and XML
   backends rely on the bundled RapidJSON and RapidXML headers; the CMake target adds them to
   your include path automatically.

.. note::

   **Boost and Qt are detected automatically.** There are no CMake options to enable or disable
   them. During configuration, ``find_package`` searches for Boost and Qt6; if found, the test
   suite is linked against them and the corresponding compile definitions are set. A status line
   is always printed so you know what was found:

   .. code-block:: none

      -- Boost found - enabling Boost serialization tests
      -- Qt6 not found - skipping Qt serialization tests

System-wide installation
~~~~~~~~~~~~~~~~~~~~~~~~~~

Build and install the library:

.. code-block:: shell

   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSERIO_BUILD_TESTS=OFF
   cmake --build build
   sudo cmake --install build

``sudo`` may or may not be required depending on the install destination; use
``-DCMAKE_INSTALL_PREFIX=<path>`` to control it. Then consume Serio from your own CMake project:

.. code-block:: cmake

   find_package(serio REQUIRED)
   add_executable(myexec main.cpp)
   target_link_libraries(myexec PRIVATE serio::serio)

As a CMake subdirectory
~~~~~~~~~~~~~~~~~~~~~~~~~

Drop the Serio source tree inside your project and add it directly:

.. code-block:: cmake

   add_subdirectory(Serio)
   add_executable(myexec main.cpp)
   target_link_libraries(myexec PRIVATE ${SERIO_LIBRARIES})
   target_include_directories(myexec PRIVATE ${SERIO_INCLUDE_DIRS})

Including and versioning
~~~~~~~~~~~~~~~~~~~~~~~~~~

A single include pulls in the whole core library:

.. code-block:: cpp

   #include <serio/serio.h>

The library version is available as preprocessor macros:

.. code-block:: cpp

   SERIO_VERSION_MAJOR
   SERIO_VERSION_MINOR
   SERIO_VERSION_PATCH

.. note::

   Avoid ``using namespace Serio;`` — the names are generic enough that it may cause conflicts.

Quick start
-----------

Every public function takes the format as a template argument (``Serio::Binary``, ``Serio::JSON``,
or ``Serio::XML``) and an options struct as its first parameter. Pass ``{}`` for default options.

.. code-block:: cpp

   #include <serio/serio.h>
   #include <vector>
   #include <string>

   std::vector<int> numbers = {1, 2, 3};
   std::string name = "serio";

   // Serialize one or more values into an in-memory buffer (std::string).
   std::string bytes = Serio::serialize<Serio::Binary>({}, numbers, name);

   // Deserialize them back, in the same order.
   std::vector<int> out_numbers;
   std::string out_name;
   Serio::deserialize<Serio::Binary>({}, bytes, out_numbers, out_name);

You can pass any number of values in a single call, and supported types nest arbitrarily
(a ``std::list<std::vector<std::pair<int, std::string>>>`` works exactly as you would expect).

Choosing a format
-----------------

The only thing that changes between formats is the template argument:

- ``Serio::Binary`` — a compact, versioned byte stream. The fastest and smallest option, and the
  only one that supports checksums, compression, and encryption. This is the default choice.
- ``Serio::JSON`` — human-readable UTF-8 JSON text. Strings are assumed to be UTF-8 text; wrap raw
  bytes with ``Serio::binaryString()`` to base64-encode them. Use ``Serio::nvp()`` to produce
  named object fields instead of positional arrays (see `Named fields (NVP)`_).
- ``Serio::XML`` — UTF-8 XML text with the same structural mapping as JSON. Prefer JSON unless an
  XML consumer is on the other end.

  .. note::

     The XML backend stores string values in XML text nodes, which imposes two limitations:

     * **XML-special characters** — the characters ``<``, ``>``, ``&``, ``"``, and ``'`` are
       XML-escaped on write (``&lt;``, ``&gt;``, ``&amp;``, ``&quot;``, ``&apos;``), but the
       underlying RapidXML parser does not unescape them on read.  Strings containing these
       characters will **not** round-trip correctly through the XML backend; use the Binary or
       JSON backend instead, or pre-escape the data yourself.
     * **Control characters** — characters such as ``\t``, ``\r``, and ``\n`` are not preserved
       in XML text nodes by RapidXML and will be silently dropped on deserialization.

.. code-block:: cpp

   std::string json = Serio::serialize<Serio::JSON>({}, numbers, name);
   std::string xml  = Serio::serialize<Serio::XML>({}, numbers, name);

The API
-------

Serio offers three pairs of functions. All of them are variadic and accept any number of
serializable values.

In-memory buffers — ``serialize`` / ``deserialize``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``serialize`` returns a ``std::string`` holding the result; ``deserialize`` reads from a
``Serio::StringView``, which a ``std::string`` (or a ``std::string_view`` on C++17) converts to
implicitly — you can also build one explicitly from a raw pointer and length.

.. code-block:: cpp

   std::string bytes = Serio::serialize<Serio::Binary>({}, value);
   Serio::deserialize<Serio::Binary>({}, bytes, value);

Files — ``save`` / ``load``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These serialize/deserialize directly to and from a file path. On Unix-like platforms ``load``
memory-maps the file for speed.

.. code-block:: cpp

   Serio::save<Serio::Binary>({}, "data.bin", value);
   Serio::load<Serio::Binary>({}, "data.bin", value);

Streams — ``write`` / ``read``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These work with any ``std::ostream`` / ``std::istream``. The streams must be **character streams
opened in binary mode**.

.. code-block:: cpp

   std::ofstream os("data.bin", std::ios::binary);
   Serio::write<Serio::Binary>({}, os, value);

   std::ifstream is("data.bin", std::ios::binary);
   Serio::read<Serio::Binary>({}, is, value);

.. note::

   **Streaming has limits.** Checksums, compression, and encryption all need the full payload
   before the header can be finalized, so they are **not** supported by ``write`` / ``read``.
   Enabling any of them in stream mode throws ``Serio::Exception``. Use ``serialize`` + ``save``
   when you need those features.

Options
~~~~~~~

``SerializeOptions`` and ``DeserializeOptions`` control the optional features. Every field has a
sensible default, so ``{}`` is always valid.

.. code-block:: cpp

   Serio::SerializeOptions sopt;
   sopt.enableChecksum  = true;       // store a CRC32 of the payload in the binary header
   sopt.compressLevel   = 10;         // zstd level 0-22, -1 disables compression
   sopt.encryptPassword = "s3cret";   // password-encrypt the binary payload
   sopt.compactFrom     = true;       // minify JSON/XML (no indentation)

   std::string bytes = Serio::serialize<Serio::Binary>(sopt, value);

   Serio::DeserializeOptions dopt;
   dopt.maxLength       = 1000000;    // reject any container whose length exceeds this
   dopt.decryptPassword = "s3cret";

   Serio::deserialize<Serio::Binary>(dopt, bytes, value);

A few notes on the options:

- When both compression and encryption are enabled, data is **compressed first, then encrypted**.
  The deserializer reverses this automatically based on flags stored in the header.
- ``enableChecksum``, ``compressLevel``, and ``encryptPassword`` only affect the **binary**
  backend; they are ignored for JSON and XML.
- ``compactFrom`` only affects **JSON and XML**; it has no effect on binary.
- ``maxLength`` is a safety guard for **untrusted input**: a malicious length field can otherwise
  trigger a huge allocation. Set it to a realistic upper bound when reading data you did not
  produce.

Supported Types
---------------

Serio supports the vast majority of the standard library. Types nest freely.

Fundamental types
~~~~~~~~~~~~~~~~~~

- All arithmetic types: ``bool``, ``char``, the integer types, and IEEE-754 ``float`` / ``double``
  (and ``long double`` where it is IEEE-754).
- ``enum`` and ``enum class`` (serialized as their underlying integer).
- 128-bit integers (``__int128``) on compilers that provide them.

.. note::

   For fully machine-independent files, prefer fixed-width integers (``int32_t``, ``uint64_t``, …)
   over types whose size varies by platform (``int``, ``long``, …).

Strings
~~~~~~~

- ``std::string`` and ``std::basic_string<T>`` (``std::wstring``, ``std::u16string``,
  ``std::u32string``).
- ``std::string_view`` / ``std::basic_string_view<T>`` *(C++17)* — **serialize-only**
  (see `Serialize-only types`_).
- ``std::filesystem::path`` *(C++17, when ``<filesystem>`` is available)*.

Sequence containers
~~~~~~~~~~~~~~~~~~~~~

- ``std::vector<T>`` (including the bit-packed ``std::vector<bool>`` specialization).
- ``std::list<T>``, ``std::deque<T>``, ``std::forward_list<T>``.
- ``std::valarray<T>``.
- ``std::array<T, N>`` — fixed size, no length prefix.
- ``std::span<T, N>`` *(C++20)* — **serialize-only**.

Associative containers
~~~~~~~~~~~~~~~~~~~~~~~~

- ``std::set``, ``std::multiset``, ``std::unordered_set``, ``std::unordered_multiset``.
- ``std::map``, ``std::multimap``, ``std::unordered_map``, ``std::unordered_multimap``.

Container adapters
~~~~~~~~~~~~~~~~~~~

- ``std::queue``, ``std::stack``, ``std::priority_queue``.

Utility and other types
~~~~~~~~~~~~~~~~~~~~~~~~~

- ``std::pair<T1, T2>`` and ``std::tuple<Ts...>``.
- ``std::complex<T>``.
- ``std::bitset<N>`` — bit-packed.
- ``std::chrono::duration`` and ``std::chrono::time_point``.
- ``std::atomic<T>``.
- ``std::shared_ptr<T>`` and ``std::unique_ptr<T>`` (a presence flag plus the pointee, when set).
- ``std::weak_ptr<T>`` — **serialize-only**.
- ``std::optional<T>`` *(C++17)*.
- ``std::variant<Ts...>`` and ``std::monostate`` *(C++17)*.
- ``std::expected<T, E>`` *(C++23)*.

Serio helper types
~~~~~~~~~~~~~~~~~~~

- ``Serio::PointerView<T>`` — a non-owning view over a raw buffer; **serialize-only**.
- ``Serio::StaticArrayView<T, N>`` — a non-owning view over a fixed-size buffer; serialize **and**
  deserialize.
- ``Serio::BinaryString<T>`` (via ``Serio::binaryString()``) — forces base64 encoding in JSON/XML.
- ``Serio::NVP<T>`` (via ``Serio::nvp()``) — named fields in JSON/XML objects.
- Any class registered with ``SERIO_REGISTER`` or adapted via ``Serio::CustomClass``.

Extensions
~~~~~~~~~~

Optional headers add support for popular third-party containers. Each header is a no-op include
guard if the required library is not present — **no CMake option is needed**; the build system
detects Boost and Qt6 automatically and enables the relevant test targets when they are found.

- ``#include <serio/qt.h>`` — ``QString``, ``QByteArray``, ``QList``, ``QVector``, ``QMap``,
  ``QHash``, ``QSet``, ``QStack``, ``QQueue``, and more. Requires Qt6 to be installed.
- ``#include <serio/boost.h>`` — Boost.Container types (``flat_map``, ``small_vector``,
  ``static_vector``, …), Boost tuples and fusion sequences, and others. Requires Boost to be
  installed.

If a standard type you need is missing, you can always make it work via
`Custom types`_.

Custom types
------------

Your own structs and classes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Add the ``SERIO_REGISTER`` macro to a class body, listing the members that should participate.
That is all that is needed — the type is now usable everywhere a built-in type is, in all three
backends.

.. code-block:: cpp

   struct Point
   {
       float x, y;
       SERIO_REGISTER(x, y)
   };

   std::vector<Point> path = {{0, 0}, {1, 1}};
   std::string bytes = Serio::serialize<Serio::Binary>({}, path);

Members are processed in the order listed, so keep that order stable across versions for binary
compatibility.

Types you do not own
~~~~~~~~~~~~~~~~~~~~~

When you cannot edit the class (it comes from a third-party library), specialize
``Serio::CustomClass`` instead. The ``process`` call lists the fields exactly like
``SERIO_REGISTER`` would.

.. code-block:: cpp

   struct Vec3 { float x, y, z; };   // not yours to modify

   namespace Serio
   {
   template <>
   struct CustomClass<Vec3>
   {
       template <typename Serializer>
       void serialize(const Vec3& v, Serializer& C) { C.process(v.x, v.y, v.z); }

       template <typename Deserializer>
       void deserialize(Vec3& v, Deserializer& C) { C.process(v.x, v.y, v.z); }
   };
   }  // namespace Serio

Backend-specific serialization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sometimes a type needs a different wire format depending on the backend — for example, binary
serialization might write a length-prefixed byte sequence while JSON must emit a proper array.
Six type aliases in the ``Serio`` namespace let you restrict a ``CustomClass`` method to a
specific backend by using it as the **return type**:

.. list-table::
   :header-rows: 1
   :widths: 28 72

   * - Alias
     - Resolves to ``void`` when …
   * - ``IfBinary<T>``
     - ``T`` is the binary serializer or deserializer.
   * - ``IfJSON<T>``
     - ``T`` is the JSON serializer or deserializer.
   * - ``IfXML<T>``
     - ``T`` is the XML serializer or deserializer.
   * - ``IfNotBinary<T>``
     - ``T`` is **not** the binary backend.
   * - ``IfNotJSON<T>``
     - ``T`` is **not** the JSON backend.
   * - ``IfNotXML<T>``
     - ``T`` is **not** the XML backend.

When the condition is false, SFINAE removes the overload from the candidate set, so the compiler
picks the other overload automatically. Provide one overload per desired variant and the right
one is selected at compile time with no runtime cost.

The example below registers a ``QSet``-like type that needs a compact binary layout (explicit
length prefix) but must call ``startArray()`` before emitting elements in JSON so the backend
can set up its array-writing state:

.. code-block:: cpp

   namespace Serio
   {
   template <typename T>
   struct CustomClass<MySet<T>>
   {
       template <typename Serializer>
       IfNotJSON<Serializer> serialize(const MySet<T>& value, Serializer& C)
       {
           C << Serio::Size(value.size());
           for (const auto& item : value) C << item;
       }
       template <typename Serializer>
       IfJSON<Serializer> serialize(const MySet<T>& value, Serializer& C)
       {
           C.startArray();
           for (const auto& item : value) C << item;
       }

       template <typename Deserializer>
       IfNotJSON<Deserializer> deserialize(MySet<T>& value, Deserializer& C)
       {
           value.clear();
           auto size = C.getLength();
           for (Serio::Size i = 0; i < size; ++i) value.insert(C.template get<T>());
       }
       template <typename Deserializer>
       IfJSON<Deserializer> deserialize(MySet<T>& value, Deserializer& C)
       {
           value.clear();
           auto size = C.startArray();
           for (Serio::Size i = 0; i < size; ++i) value.insert(C.template get<T>());
       }
   };
   }  // namespace Serio

.. note::

   ``IfNotJSON`` matches **both** Binary and XML. If you need all three backends to behave
   differently, combine ``IfBinary``, ``IfJSON``, and ``IfXML`` instead.

Custom containers
~~~~~~~~~~~~~~~~~

For a container with a variable number of elements, write ``_serialize`` / ``_deserialize`` by
hand and add ``SERIO_FRIEND`` so the internal machinery can reach private members. Serialize the
element count as a ``Serio::Size`` first (a fixed 64-bit type, so the data stays portable), then
the elements:

.. code-block:: cpp

   class MyBuffer
   {
       std::vector<int> data;

       SERIO_FRIEND;

       template <typename Serializer>
       void _serialize(Serializer& C) const
       {
           C << Serio::Size(data.size());
           for (const auto& v : data) C << v;
       }
       template <typename Deserializer>
       void _deserialize(Deserializer& C)
       {
           Serio::Size size;
           C >> size;
           data.resize(size_t(size));
           for (auto& v : data) C >> v;
       }
   };

Named fields (NVP)
~~~~~~~~~~~~~~~~~~~

By default a registered type is written as a positional array in JSON/XML. To emit a JSON object
(or named XML elements) instead, wrap each member with ``Serio::nvp()`` inside a hand-written
``_serialize`` / ``_deserialize`` pair:

.. code-block:: cpp

   struct Config
   {
       bool   enabled;
       int    retries;
       std::string host;

       SERIO_FRIEND;

       template <typename Serializer>
       void _serialize(Serializer& C) const
       {
           C.process(Serio::nvp("enabled", enabled),
                     Serio::nvp("retries", retries),
                     Serio::nvp("host", host));
       }
       template <typename Deserializer>
       void _deserialize(Deserializer& C)
       {
           C.process(Serio::nvp("enabled", enabled),
                     Serio::nvp("retries", retries),
                     Serio::nvp("host", host));
       }
   };

   // {"enabled": true, "retries": 3, "host": "localhost"}
   std::string json = Serio::serialize<Serio::JSON>({}, Config{true, 3, "localhost"});

Binary strings in JSON/XML
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

JSON and XML assume strings are UTF-8 text. If a string holds arbitrary bytes (a blob, a key,
image data), wrap it with ``Serio::binaryString()`` so it is base64-encoded and round-trips
safely. It is a no-op in the binary backend.

.. code-block:: cpp

   std::string blob = load_some_bytes();
   std::string json = Serio::serialize<Serio::JSON>({}, Serio::binaryString(blob));

   std::string restored;
   Serio::deserialize<Serio::JSON>({}, json, Serio::binaryString(restored));

Raw arrays and views
--------------------

To serialize a raw C array or an existing buffer without copying it into a container first, use
one of the view types:

- ``Serio::StaticArrayView<T, N>`` wraps a buffer whose length ``N`` is known at compile time. It
  works in **both** directions — no length prefix is written (just like ``std::array``), and on
  deserialization the bytes are written straight into your buffer.

  .. code-block:: cpp

     int a[10] = { /* ... */ };
     std::string bytes = Serio::serialize<Serio::Binary>({}, Serio::StaticArrayView<int, 10>(a));

     int b[10];
     Serio::deserialize<Serio::Binary>({}, bytes, Serio::StaticArrayView<int, 10>(b));

- ``Serio::PointerView<T>`` wraps a pointer + element count for buffers whose size is only known
  at runtime. It is **serialize-only** — it has no way to allocate, so deserialize into an owning
  container (such as ``std::vector<T>``) instead.

  .. code-block:: cpp

     float* buffer = /* ... */;
     std::size_t count = /* ... */;
     std::string bytes = Serio::serialize<Serio::Binary>({}, Serio::PointerView<float>(buffer, count));

     std::vector<float> restored;   // PointerView writes a count prefix, so a vector reads it back
     Serio::deserialize<Serio::Binary>({}, bytes, restored);

Interchangeable containers and exceptions
-----------------------------------------

Serio's binary format describes data structurally, not by concrete type, so containers that share
the same on-the-wire shape can be **freely swapped between serialization and deserialization**.
This is convenient and sometimes faster. There are a few important exceptions — read these
carefully.

Containers you can swap
~~~~~~~~~~~~~~~~~~~~~~~~

Resizable sequences all share the layout ``[count][elements…]``, so any of them can be
deserialized from any other (as long as the element type matches):

.. code-block:: cpp

   // serialize as a vector...
   auto bytes = Serio::serialize<Serio::Binary>({}, std::vector<int>{3, 1, 2, 1});

   std::list<int> as_list;   // ...read back as a list
   Serio::deserialize<Serio::Binary>({}, bytes, as_list);

   std::set<int> as_set;     // ...or as a set, to get sorted, unique items: {1, 2, 3}
   Serio::deserialize<Serio::Binary>({}, bytes, as_set);

This covers ``std::vector``, ``std::list``, ``std::deque``, ``std::forward_list``,
``std::valarray``, and the set/map families. Because maps serialize their elements as key/value
**pairs**, you can also round-trip between a map and a sequence of pairs:

.. code-block:: cpp

   auto bytes = Serio::serialize<Serio::Binary>({}, std::map<int, int>{{1, 10}, {2, 20}});

   std::vector<std::pair<int, int>> as_pairs;
   Serio::deserialize<Serio::Binary>({}, bytes, as_pairs);   // works, and vice-versa

Fixed-size containers are different
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``std::array<T, N>``, ``Serio::StaticArrayView<T, N>``, and fixed-extent ``std::span<T, N>``
write **no length prefix** (their size is part of the type). They are therefore only
interchangeable **with each other** (at the same ``N``), and **not** with the resizable
containers above. Mixing the two leads to a corrupted read.

``std::vector<bool>`` and ``std::bitset`` are bit-packed
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To save space, ``std::vector<bool>`` and ``std::bitset<N>`` are stored as packed bits rather than
one element per boolean. As a result:

- A ``std::vector<bool>`` can only be deserialized as another ``std::vector<bool>`` — **not** as
  ``std::list<bool>`` or ``std::deque<bool>``.
- ``std::bitset<N>`` has its own fixed bit-packed format.

Serialize-only types
~~~~~~~~~~~~~~~~~~~~~~

Some types can be **written but not read back**, because they are non-owning views or cannot
reconstruct their state. Attempting to deserialize into one of them is a **compile-time error**.
Serialize from them, and deserialize into an owning type instead:

.. list-table::
   :header-rows: 1
   :widths: 50 50

   * - Serialize-only type
     - Deserialize into
   * - ``Serio::PointerView<T>``
     - ``std::vector<T>`` (or any resizable sequence)
   * - ``std::weak_ptr<T>``
     - ``std::shared_ptr<T>``
   * - ``std::string_view`` / ``std::basic_string_view``
     - ``std::string`` / ``std::basic_string<T>``
   * - ``std::span<T, N>``
     - ``std::array<T, N>`` / ``StaticArrayView<T, N>``

Error handling
--------------

Every runtime error Serio detects — malformed data, a version mismatch, a failed CRC check, a
type mismatch in JSON/XML, an I/O failure, a container length over ``maxLength`` — is reported by
throwing ``Serio::Exception`` (derived from ``std::exception``). Catch it (or ``std::exception``)
to handle all library errors uniformly:

.. code-block:: cpp

   try
   {
       Serio::deserialize<Serio::Binary>(dopt, untrusted_bytes, value);
   }
   catch (const Serio::Exception& e)
   {
       std::cerr << "Deserialization failed: " << e.what() << '\n';
   }

A common compile-time error is::

   error: no member named '_serialize' in '<class-name>'
   error: no member named '_deserialize' in '<class-name>'

This means you tried to serialize a custom class that was never registered — add
``SERIO_REGISTER`` to it or specialize ``Serio::CustomClass`` for it (see `Custom types`_).

Version compatibility
---------------------

The binary backend writes a small versioned header. On read, Serio requires the **major** version
to match exactly and the data's **minor** version to be no newer than the library's own. In other
words, a newer library can always read data produced by an older library at the same major
version. Incompatible data throws ``Serio::Exception``.
