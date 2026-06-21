#include <filesystem>

#include "common.h"

// ---- Checksum ----

TEST(Options, ChecksumEnabled)
{
    Serio::SerializeOptions sopt;
    sopt.enableChecksum = true;
    auto bytes = Serio::serialize<Serio::Binary>(sopt, std::vector<int>{1, 2, 3});
    // Bytes with checksum should differ from without
    auto bytes_no_crc = Serio::serialize<Serio::Binary>({}, std::vector<int>{1, 2, 3});
    EXPECT_NE(bytes, bytes_no_crc);
}

TEST(Options, ChecksumRoundtrip)
{
    Serio::SerializeOptions sopt;
    sopt.enableChecksum = true;
    std::vector<int> orig{10, 20, 30, 40};
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, orig);
}

TEST(Options, ChecksumRoundtripString)
{
    Serio::SerializeOptions sopt;
    sopt.enableChecksum = true;
    std::string orig = "hello checksum world";
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    std::string out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, orig);
}

TEST(Options, ChecksumDetectsCorruption)
{
    Serio::SerializeOptions sopt;
    sopt.enableChecksum = true;
    auto bytes = Serio::serialize<Serio::Binary>(sopt, std::vector<int>{1, 2, 3});
    // Corrupt a byte in the payload
    bytes[bytes.size() / 2] ^= 0xFF;
    std::vector<int> out;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>({}, bytes, out), Serio::Exception);
}

// ---- Compression ----

TEST(Options, CompressLevel1)
{
    Serio::SerializeOptions sopt;
    sopt.compressLevel = 1;
    std::vector<int> orig(1000, 42);
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, orig);
}

TEST(Options, CompressLevel10)
{
    Serio::SerializeOptions sopt;
    sopt.compressLevel = 10;
    std::string orig(5000, 'A');
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    // Compressed should be smaller than uncompressed
    auto uncompressed = Serio::serialize<Serio::Binary>({}, orig);
    EXPECT_LT(bytes.size(), uncompressed.size());
    std::string out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, orig);
}

TEST(Options, CompressLevel22)
{
    Serio::SerializeOptions sopt;
    sopt.compressLevel = 22;
    std::vector<int> orig(500, 0);
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, orig);
}

TEST(Options, CompressLevelDisabled)
{
    Serio::SerializeOptions sopt;
    sopt.compressLevel = -1;  // disabled
    std::vector<int> orig{1, 2, 3};
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    auto bytes_default = Serio::serialize<Serio::Binary>({}, orig);
    EXPECT_EQ(bytes, bytes_default);
}

TEST(Options, CompressRoundtripCustomType)
{
    Serio::SerializeOptions sopt;
    sopt.compressLevel = 5;
    std::vector<Point2D> orig(100, {1.0f, 2.0f});
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    std::vector<Point2D> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, orig);
}

// ---- Encryption ----

TEST(Options, EncryptDecrypt)
{
    Serio::SerializeOptions sopt;
    sopt.encryptPassword = "s3cr3t";
    Serio::DeserializeOptions dopt;
    dopt.decryptPassword = "s3cr3t";

    std::vector<int> orig{1, 2, 3, 4, 5};
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>(dopt, bytes, out);
    EXPECT_EQ(out, orig);
}

TEST(Options, EncryptProducesDifferentBytes)
{
    Serio::SerializeOptions sopt;
    sopt.encryptPassword = "password";
    std::vector<int> v{1, 2, 3};
    auto enc = Serio::serialize<Serio::Binary>(sopt, v);
    auto plain = Serio::serialize<Serio::Binary>({}, v);
    EXPECT_NE(enc, plain);
}

TEST(Options, EncryptWrongPassword)
{
    Serio::SerializeOptions sopt;
    sopt.encryptPassword = "correct_password";
    auto bytes = Serio::serialize<Serio::Binary>(sopt, std::string("secret"));
    Serio::DeserializeOptions dopt;
    dopt.decryptPassword = "wrong_password";
    std::string out;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>(dopt, bytes, out), Serio::Exception);
}

TEST(Options, EncryptWithCompression)
{
    Serio::SerializeOptions sopt;
    sopt.compressLevel = 5;
    sopt.encryptPassword = "combo";
    Serio::DeserializeOptions dopt;
    dopt.decryptPassword = "combo";

    std::string orig(2000, 'X');
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    std::string out;
    Serio::deserialize<Serio::Binary>(dopt, bytes, out);
    EXPECT_EQ(out, orig);
}

TEST(Options, EncryptWithChecksum)
{
    Serio::SerializeOptions sopt;
    sopt.enableChecksum = true;
    sopt.encryptPassword = "crc_enc";
    Serio::DeserializeOptions dopt;
    dopt.decryptPassword = "crc_enc";

    std::vector<double> orig{1.1, 2.2, 3.3};
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    std::vector<double> out;
    Serio::deserialize<Serio::Binary>(dopt, bytes, out);
    EXPECT_EQ(out, orig);
}

TEST(Options, EncryptAllThree)
{
    Serio::SerializeOptions sopt;
    sopt.enableChecksum = true;
    sopt.compressLevel = 3;
    sopt.encryptPassword = "all_three";
    Serio::DeserializeOptions dopt;
    dopt.decryptPassword = "all_three";

    std::map<std::string, std::vector<int>> orig{{"a", {1, 2}}, {"b", {3, 4, 5}}};
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    std::map<std::string, std::vector<int>> out;
    Serio::deserialize<Serio::Binary>(dopt, bytes, out);
    EXPECT_EQ(out, orig);
}

TEST(Options, EncryptEmptyPayload)
{
    Serio::SerializeOptions sopt;
    sopt.encryptPassword = "pw";
    Serio::DeserializeOptions dopt;
    dopt.decryptPassword = "pw";

    std::vector<int> orig;
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>(dopt, bytes, out);
    EXPECT_EQ(out, orig);
}

TEST(Options, EncryptStringPassword)
{
    Serio::SerializeOptions sopt;
    sopt.encryptPassword = std::string(256, 'k');  // long password
    Serio::DeserializeOptions dopt;
    dopt.decryptPassword = std::string(256, 'k');

    int orig = 42;
    auto bytes = Serio::serialize<Serio::Binary>(sopt, orig);
    int out = 0;
    Serio::deserialize<Serio::Binary>(dopt, bytes, out);
    EXPECT_EQ(out, orig);
}

// ---- maxLength ----

TEST(Options, MaxLengthAccepted)
{
    Serio::SerializeOptions sopt;
    Serio::DeserializeOptions dopt;
    dopt.maxLength = 100;
    std::vector<int> v(10, 0);  // 10 elements, under limit
    auto bytes = Serio::serialize<Serio::Binary>(sopt, v);
    std::vector<int> out;
    EXPECT_NO_THROW(Serio::deserialize<Serio::Binary>(dopt, bytes, out));
    EXPECT_EQ(out, v);
}

TEST(Options, MaxLengthRejected)
{
    Serio::SerializeOptions sopt;
    Serio::DeserializeOptions dopt;
    dopt.maxLength = 5;
    std::vector<int> v(10, 0);  // 10 elements, over limit
    auto bytes = Serio::serialize<Serio::Binary>(sopt, v);
    std::vector<int> out;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>(dopt, bytes, out), Serio::Exception);
}

TEST(Options, MaxLengthExact)
{
    Serio::SerializeOptions sopt;
    Serio::DeserializeOptions dopt;
    dopt.maxLength = 5;
    std::vector<int> v(5, 0);  // exactly at limit
    auto bytes = Serio::serialize<Serio::Binary>(sopt, v);
    std::vector<int> out;
    EXPECT_NO_THROW(Serio::deserialize<Serio::Binary>(dopt, bytes, out));
}

TEST(Options, MaxLengthOneRejectsTwo)
{
    // maxLength = 0 means disabled/no-limit in this library; use 1 to reject 2 elements
    Serio::SerializeOptions sopt;
    Serio::DeserializeOptions dopt;
    dopt.maxLength = 1;
    std::vector<int> v{1, 2};  // 2 elements, exceeds limit of 1
    auto bytes = Serio::serialize<Serio::Binary>(sopt, v);
    std::vector<int> out;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>(dopt, bytes, out), Serio::Exception);
}

TEST(Options, MaxLengthString)
{
    Serio::SerializeOptions sopt;
    Serio::DeserializeOptions dopt;
    dopt.maxLength = 3;
    std::string s = "abcd";  // 4 chars, over limit
    auto bytes = Serio::serialize<Serio::Binary>(sopt, s);
    std::string out;
    EXPECT_THROW(Serio::deserialize<Serio::Binary>(dopt, bytes, out), Serio::Exception);
}

// ---- compactFrom (JSON/XML) ----

TEST(Options, CompactJson)
{
    Serio::SerializeOptions sopt;
    sopt.compactFrom = true;
    auto compact = Serio::serialize<Serio::JSON>(sopt, std::vector<int>{1, 2, 3});
    auto pretty = Serio::serialize<Serio::JSON>({}, std::vector<int>{1, 2, 3});
    // compact should not contain newlines/indentation
    EXPECT_EQ(compact.find('\n'), std::string::npos);
    // Round-trip still works
    std::vector<int> out;
    Serio::deserialize<Serio::JSON>({}, compact, out);
    EXPECT_EQ(out, (std::vector<int>{1, 2, 3}));
}

TEST(Options, CompactXml)
{
    Serio::SerializeOptions sopt;
    sopt.compactFrom = true;
    auto compact = Serio::serialize<Serio::XML>(sopt, std::vector<int>{1, 2, 3});
    // Compact XML should be shorter than non-compact
    auto pretty = Serio::serialize<Serio::XML>({}, std::vector<int>{1, 2, 3});
    EXPECT_LE(compact.size(), pretty.size());
    // Round-trip works
    std::vector<int> out;
    Serio::deserialize<Serio::XML>({}, compact, out);
    EXPECT_EQ(out, (std::vector<int>{1, 2, 3}));
}

// ---- Stream mode rejects checksum/compress/encrypt ----

TEST(Options, StreamWithChecksumThrows)
{
    Serio::SerializeOptions sopt;
    sopt.enableChecksum = true;
    std::ostringstream oss;
    EXPECT_THROW(Serio::write<Serio::Binary>(sopt, oss, 42), Serio::Exception);
}

TEST(Options, StreamWithCompressionThrows)
{
    Serio::SerializeOptions sopt;
    sopt.compressLevel = 5;
    std::ostringstream oss;
    EXPECT_THROW(Serio::write<Serio::Binary>(sopt, oss, 42), Serio::Exception);
}

TEST(Options, StreamWithEncryptionThrows)
{
    Serio::SerializeOptions sopt;
    sopt.encryptPassword = "pw";
    std::ostringstream oss;
    EXPECT_THROW(Serio::write<Serio::Binary>(sopt, oss, 42), Serio::Exception);
}
