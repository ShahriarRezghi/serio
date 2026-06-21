#include "common.h"

// ---- std::string ----

TEST(BinaryStrings, StringEmpty)
{
    std::string s;
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, StringSimple) { EXPECT_EQ(roundtrip_binary(std::string("hello")), std::string("hello")); }

TEST(BinaryStrings, StringWithSpaces)
{
    EXPECT_EQ(roundtrip_binary(std::string("hello world")), std::string("hello world"));
}

TEST(BinaryStrings, StringWithNewlines)
{
    std::string s = "line1\nline2\r\nline3\t";
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, StringWithNulBytes)
{
    std::string s("ab\0cd", 5);
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, StringLong)
{
    std::string s(10000, 'x');
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, StringAllAscii)
{
    std::string s;
    for (int i = 1; i < 128; ++i) s += static_cast<char>(i);
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, StringAllBytes)
{
    std::string s;
    for (int i = 0; i < 256; ++i) s += static_cast<char>(i);
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, StringUtf8)
{
    // literal bytes for: Hello, 世界! こんにちは Привет мир
    std::string s = "Hello, \xE4\xB8\x96\xE7\x95\x8C!";
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, StringEmoji)
{
    // UTF-8 bytes for 🎉 (U+1F389)
    std::string s = "\xF0\x9F\x8E\x89\xF0\x9F\x9A\x80\xF0\x9F\x8C\x8D";
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, MultipleStrings)
{
    std::string a = "alpha", b = "beta", c = "gamma";
    auto bytes = Serio::serialize<Serio::Binary>({}, a, b, c);
    std::string oa, ob, oc;
    Serio::deserialize<Serio::Binary>({}, bytes, oa, ob, oc);
    EXPECT_EQ(oa, a);
    EXPECT_EQ(ob, b);
    EXPECT_EQ(oc, c);
}

// ---- std::wstring ----

TEST(BinaryStrings, WstringEmpty)
{
    std::wstring s;
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, WstringHello)
{
    std::wstring s = L"hello";
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, WstringUnicode)
{
    std::wstring s = L"世界";  // 世界
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, WstringMax)
{
    std::wstring s(5000, L'W');
    EXPECT_EQ(roundtrip_binary(s), s);
}

// ---- std::u16string ----

TEST(BinaryStrings, U16StringEmpty)
{
    std::u16string s;
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, U16StringSimple)
{
    std::u16string s = u"hello world";
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, U16StringUnicode)
{
    std::u16string s = u"世界こんにちは";
    EXPECT_EQ(roundtrip_binary(s), s);
}

// ---- std::u32string ----

TEST(BinaryStrings, U32StringEmpty)
{
    std::u32string s;
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, U32StringSimple)
{
    std::u32string s = U"hello";
    EXPECT_EQ(roundtrip_binary(s), s);
}

TEST(BinaryStrings, U32StringUnicode)
{
    std::u32string s = U"\U0001F600\U0001F601\U0001F602";  // emoji
    EXPECT_EQ(roundtrip_binary(s), s);
}

// ---- std::string_view (serialize-only) ----

TEST(BinaryStrings, StringViewSimple)
{
    std::string original = "hello from view";
    std::string_view sv = original;
    auto bytes = Serio::serialize<Serio::Binary>({}, sv);
    std::string out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, original);
}

TEST(BinaryStrings, StringViewEmpty)
{
    std::string_view sv;
    auto bytes = Serio::serialize<Serio::Binary>({}, sv);
    std::string out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, "");
}

TEST(BinaryStrings, StringViewSubstr)
{
    std::string original = "hello world";
    std::string_view sv(original.data() + 6, 5);  // "world"
    auto bytes = Serio::serialize<Serio::Binary>({}, sv);
    std::string out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, "world");
}

// ---- std::wstring_view (serialize-only) ----

TEST(BinaryStrings, WstringViewSimple)
{
    std::wstring original = L"wide view";
    std::wstring_view sv = original;
    auto bytes = Serio::serialize<Serio::Binary>({}, sv);
    std::wstring out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, original);
}

// ---- std::filesystem::path ----

TEST(BinaryStrings, PathSimple)
{
    std::filesystem::path p("/usr/local/bin");
    EXPECT_EQ(roundtrip_binary(p), p);
}

TEST(BinaryStrings, PathEmpty)
{
    std::filesystem::path p;
    EXPECT_EQ(roundtrip_binary(p), p);
}

TEST(BinaryStrings, PathRelative)
{
    std::filesystem::path p("relative/path/to/file.txt");
    EXPECT_EQ(roundtrip_binary(p), p);
}

TEST(BinaryStrings, PathWithSpaces)
{
    std::filesystem::path p("/path/with spaces/and unicode/世界");
    EXPECT_EQ(roundtrip_binary(p), p);
}

TEST(BinaryStrings, PathRoot)
{
    std::filesystem::path p("/");
    EXPECT_EQ(roundtrip_binary(p), p);
}

// ---- String in containers ----

TEST(BinaryStrings, VectorOfStrings)
{
    std::vector<std::string> v{"one", "two", "three", "", "five"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryStrings, MapStringToString)
{
    std::map<std::string, std::string> m{{"key1", "val1"}, {"key2", "val2"}};
    EXPECT_EQ(roundtrip_binary(m), m);
}
