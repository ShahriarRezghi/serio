#include <filesystem>
#include <string>

#include "common.h"

// ---- std::string ----

TEST(String, Empty) { check(std::string{}); }
TEST(String, Ascii) { check(std::string{"hello, world"}); }
TEST(String, Long) { check(std::string(1000, 'x')); }
TEST(String, Spaces) { check(std::string{"foo bar baz"}); }
TEST(String, Newlines) { check(std::string{"line1\nline2\nline3"}); }
TEST(String, Unicode) { check(std::string{"\xc3\xa9\xc3\xa0\xc3\xbc"}); }  // UTF-8 é, à, ü

// Strings with embedded null bytes - only binary (JSON/XML can't handle raw nulls in strings)
TEST(String, NullBytes_Binary)
{
    std::string v("\x00\x01\x02\x03", 4);
    check_bin(v);
}

TEST(String, HighBytes_Binary)
{
    std::string v("\xff\xfe\x80\x81\x82", 5);
    check_bin(v);
}

// ---- std::wstring ----

TEST(WString, Empty) { check(std::wstring{}); }
TEST(WString, Ascii) { check(std::wstring{L"hello"}); }
TEST(WString, Mixed) { check(std::wstring{L"ABC XYZ 123"}); }

// ---- std::u16string ----

TEST(U16String, Empty) { check(std::u16string{}); }
TEST(U16String, Ascii) { check(std::u16string{u"hello"}); }

// ---- std::u32string ----

TEST(U32String, Empty) { check(std::u32string{}); }
TEST(U32String, Ascii) { check(std::u32string{U"hello"}); }

// ---- basic_string<char> edge cases ----

TEST(String, SingleChar) { check(std::string{"a"}); }

// XML doesn't round-trip XML-special characters (<>&"') or control chars (\t\r\n)
// via its text-node path; test binary and JSON only.
TEST(String, SpecialChars)
{
    std::string v{"<>&\"'\\"};
    EXPECT_EQ(v, mem_rt<Serio::Binary>(v));
    EXPECT_EQ(v, mem_rt<Serio::JSON>(v));
    EXPECT_EQ(v, stream_rt<Serio::Binary>(v));
    EXPECT_EQ(v, stream_rt<Serio::JSON>(v));
    EXPECT_EQ(v, file_rt<Serio::Binary>(v));
    EXPECT_EQ(v, file_rt<Serio::JSON>(v));
}

TEST(String, TabsAndReturns)
{
    std::string v{"\t\r\n"};
    EXPECT_EQ(v, mem_rt<Serio::Binary>(v));
    EXPECT_EQ(v, mem_rt<Serio::JSON>(v));
    EXPECT_EQ(v, stream_rt<Serio::Binary>(v));
    EXPECT_EQ(v, stream_rt<Serio::JSON>(v));
    EXPECT_EQ(v, file_rt<Serio::Binary>(v));
    EXPECT_EQ(v, file_rt<Serio::JSON>(v));
}

// ---- std::string_view (serialize-only) ----

TEST(StringView, SerializeOnly)
{
    std::string original = "hello from string_view";
    std::string_view sv{original};

    auto bytes = Serio::serialize<Serio::Binary>({}, sv);
    std::string result;
    Serio::deserialize<Serio::Binary>({}, bytes, result);
    EXPECT_EQ(original, result);
}

TEST(StringView, EmptySerializeOnly)
{
    std::string_view sv{};
    auto bytes = Serio::serialize<Serio::Binary>({}, sv);
    std::string result;
    Serio::deserialize<Serio::Binary>({}, bytes, result);
    EXPECT_EQ("", result);
}

TEST(StringView, JSON_SerializeOnly)
{
    std::string original = "json string view test";
    std::string_view sv{original};
    auto bytes = Serio::serialize<Serio::JSON>({}, sv);
    std::string result;
    Serio::deserialize<Serio::JSON>({}, bytes, result);
    EXPECT_EQ(original, result);
}

// ---- std::filesystem::path ----

TEST(FilesystemPath, Empty)
{
    std::filesystem::path p{};
    check(p);
}

TEST(FilesystemPath, RelativePath)
{
    std::filesystem::path p{"some/relative/path"};
    check(p);
}

TEST(FilesystemPath, AbsolutePath)
{
    std::filesystem::path p{"/usr/local/include"};
    check(p);
}

TEST(FilesystemPath, Filename)
{
    std::filesystem::path p{"file.txt"};
    check(p);
}

TEST(FilesystemPath, PathWithSpaces)
{
    std::filesystem::path p{"path with spaces/file name.txt"};
    check(p);
}

TEST(FilesystemPath, DotDot)
{
    std::filesystem::path p{"../parent/child"};
    check(p);
}

// ---- Nested: vector of strings ----

TEST(String, VectorOfStrings)
{
    std::vector<std::string> v{"hello", "world", "", "foo"};
    check(v);
}

TEST(String, VectorOfWStrings)
{
    std::vector<std::wstring> v{L"hello", L"world", L""};
    check(v);
}
