#include <filesystem>

#include "common.h"

// ---- Serio::Exception is std::exception ----

TEST(Errors, ExceptionInheritance)
{
    // Serio::Exception should be catchable as std::exception
    bool caught = false;
    try
    {
        std::string bad = "not valid binary serio data";
        int v;
        Serio::deserialize<Serio::Binary>({}, bad, v);
    }
    catch (const std::exception& e)
    {
        caught = true;
        EXPECT_FALSE(std::string(e.what()).empty());
    }
    EXPECT_TRUE(caught);
}

TEST(Errors, ExceptionCatchByType)
{
    bool caught = false;
    try
    {
        std::string bad = "garbage";
        int v;
        Serio::deserialize<Serio::Binary>({}, bad, v);
    }
    catch (const Serio::Exception& e)
    {
        caught = true;
        EXPECT_FALSE(std::string(e.what()).empty());
    }
    EXPECT_TRUE(caught);
}

// ---- Malformed binary data ----

TEST(Errors, EmptyInputThrows)
{
    std::string empty;
    int v;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>({}, empty, v), Serio::Exception);
}

TEST(Errors, TruncatedDataThrows)
{
    // Serialize a vector, truncate it, try to deserialize
    std::vector<int> orig{1, 2, 3, 4, 5};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::string truncated = bytes.substr(0, bytes.size() / 2);
    std::vector<int> out;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>({}, truncated, out), Serio::Exception);
}

TEST(Errors, RandomGarbageThrows)
{
    // Random bytes very unlikely to be a valid serio header
    std::string garbage(50, '\xAB');
    std::vector<int> out;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>({}, garbage, out), Serio::Exception);
}

TEST(Errors, OnlyHeaderNoPayloadThrows)
{
    // Serialize something, keep just the header portion
    std::string orig = Serio::serialize<Serio::Binary>({}, std::vector<int>{1, 2, 3});
    // Keep only first few bytes (header likely < 16 bytes)
    std::string truncated = orig.substr(0, std::min(orig.size(), (std::size_t)8));
    std::vector<int> out;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>({}, truncated, out), Serio::Exception);
}

// ---- Checksum failure ----

TEST(Errors, CorruptedChecksumThrows)
{
    Serio::SerializeOptions sopt;
    sopt.enableChecksum = true;
    auto bytes = Serio::serialize<Serio::Binary>(sopt, std::vector<int>{1, 2, 3});
    // Flip a byte in the payload area
    bytes[bytes.size() - 1] ^= 0xFF;
    std::vector<int> out;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>({}, bytes, out), Serio::Exception);
}

// ---- Wrong password ----

TEST(Errors, WrongDecryptPasswordThrows)
{
    Serio::SerializeOptions sopt;
    sopt.encryptPassword = "correct";
    auto bytes = Serio::serialize<Serio::Binary>(sopt, 42);
    Serio::DeserializeOptions dopt;
    dopt.decryptPassword = "wrong";
    int out;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>(dopt, bytes, out), Serio::Exception);
}

TEST(Errors, MissingPasswordWhenEncrypted)
{
    Serio::SerializeOptions sopt;
    sopt.encryptPassword = "secret";
    auto bytes = Serio::serialize<Serio::Binary>(sopt, 42);
    // No password provided for decryption
    int out;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>({}, bytes, out), Serio::Exception);
}

// ---- maxLength exceeded ----

TEST(Errors, MaxLengthExceededThrows)
{
    std::vector<int> big(1000, 0);
    auto bytes = Serio::serialize<Serio::Binary>({}, big);
    Serio::DeserializeOptions dopt;
    dopt.maxLength = 10;
    std::vector<int> out;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>(dopt, bytes, out), Serio::Exception);
}

// ---- File not found ----

TEST(Errors, LoadNonExistentFileThrows)
{
    int v;
    EXPECT_THROW(Serio::load<Serio::Binary>({}, "/nonexistent/path/file.bin", v), Serio::Exception);
}

// ---- Stream read past EOF ----

TEST(Errors, ReadFromEmptyStreamThrows)
{
    std::istringstream iss("", std::ios::binary);
    int v;
    EXPECT_THROW(Serio::read<Serio::Binary>({}, iss, v), Serio::Exception);
}

TEST(Errors, ReadTruncatedStreamThrows)
{
    std::string bytes = Serio::serialize<Serio::Binary>({}, std::vector<int>{1, 2, 3, 4, 5});
    // Only put half the bytes in the stream
    std::istringstream iss(bytes.substr(0, bytes.size() / 2), std::ios::binary);
    std::vector<int> out;
    EXPECT_THROW(Serio::read<Serio::Binary>({}, iss, out), Serio::Exception);
}

// ---- Stream mode with incompatible options ----

TEST(Errors, StreamWriteWithChecksumThrows)
{
    Serio::SerializeOptions sopt;
    sopt.enableChecksum = true;
    std::ostringstream oss;
    EXPECT_THROW(Serio::write<Serio::Binary>(sopt, oss, 42), Serio::Exception);
}

TEST(Errors, StreamWriteWithCompressionThrows)
{
    Serio::SerializeOptions sopt;
    sopt.compressLevel = 5;
    std::ostringstream oss;
    EXPECT_THROW(Serio::write<Serio::Binary>(sopt, oss, 42), Serio::Exception);
}

TEST(Errors, StreamWriteWithEncryptionThrows)
{
    Serio::SerializeOptions sopt;
    sopt.encryptPassword = "pw";
    std::ostringstream oss;
    EXPECT_THROW(Serio::write<Serio::Binary>(sopt, oss, 42), Serio::Exception);
}

// ---- JSON parse errors ----

TEST(Errors, JsonGarbageThrows)
{
    std::string bad = "not json at all !!!";
    int v;
    EXPECT_THROW(Serio::deserialize<Serio::JSON>({}, bad, v), Serio::Exception);
}

TEST(Errors, JsonEmptyThrows)
{
    std::string empty;
    int v;
    EXPECT_THROW(Serio::deserialize<Serio::JSON>({}, empty, v), Serio::Exception);
}

TEST(Errors, JsonTypeMismatchThrows)
{
    // Serialize an int, try to deserialize as vector
    auto json = Serio::serialize<Serio::JSON>({}, 42);
    std::vector<int> v;
    EXPECT_THROW(Serio::deserialize<Serio::JSON>({}, json, v), Serio::Exception);
}

// ---- XML parse errors ----

TEST(Errors, XmlGarbageThrows)
{
    std::string bad = "not xml at all";
    int v;
    EXPECT_THROW(Serio::deserialize<Serio::XML>({}, bad, v), Serio::Exception);
}

TEST(Errors, XmlEmptyThrows)
{
    std::string empty;
    int v;
    EXPECT_THROW(Serio::deserialize<Serio::XML>({}, empty, v), Serio::Exception);
}

// ---- Exception what() message is non-empty ----

TEST(Errors, ExceptionHasMessage)
{
    std::string bad = "garbage data";
    int v;
    try
    {
        Serio::deserialize<Serio::Binary>({}, bad, v);
        FAIL() << "Expected exception";
    }
    catch (const Serio::Exception& e)
    {
        EXPECT_GT(std::string(e.what()).size(), 0u);
    }
}

// ---- Valid data doesn't throw ----

TEST(Errors, ValidDataNoThrow)
{
    auto bytes = Serio::serialize<Serio::Binary>({}, std::vector<int>{1, 2, 3});
    std::vector<int> out;
    EXPECT_NO_THROW(Serio::deserialize<Serio::Binary>({}, bytes, out));
}

TEST(Errors, ValidJsonNoThrow)
{
    auto json = Serio::serialize<Serio::JSON>({}, std::string("hello"));
    std::string out;
    EXPECT_NO_THROW(Serio::deserialize<Serio::JSON>({}, json, out));
}

TEST(Errors, ValidXmlNoThrow)
{
    auto xml = Serio::serialize<Serio::XML>({}, int(42));
    int out;
    EXPECT_NO_THROW(Serio::deserialize<Serio::XML>({}, xml, out));
}
