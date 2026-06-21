#include "common.h"

#ifdef SERIO_TEST_QT

#include <serio/qt.h>

// ---- QChar ----

TEST(QtChar, AsciiLetter)
{
    QChar v('A');
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QChar out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtChar, NullChar)
{
    QChar v(QChar::Null);
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QChar out('Z');
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtChar, HighUnicode)
{
    QChar v(0x4E2D);  // CJK character
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QChar out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtChar, GreekAlpha) { check(QChar(0x03B1)); }

TEST(QtChar, JsonRoundtrip)
{
    QChar v('B');
    auto json = Serio::serialize<Serio::JSON>({}, v);
    QChar out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_EQ(out, v);
}

// ---- QString ----

TEST(QtString, Empty)
{
    QString v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QString out("placeholder");
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtString, Simple)
{
    QString v("hello world");
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QString out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtString, Long)
{
    QString v(1000, QChar('x'));
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QString out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtString, Unicode) { check(QString::fromUtf8("café")); }

TEST(QtString, JsonRoundtrip)
{
    QString v("json test");
    auto json = Serio::serialize<Serio::JSON>({}, v);
    QString out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_EQ(out, v);
}

// ---- QByteArray ----

TEST(QtByteArray, Empty)
{
    QByteArray v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QByteArray out("placeholder");
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtByteArray, SimpleAscii)
{
    QByteArray v("hello");
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QByteArray out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtByteArray, BinaryData)
{
    QByteArray v;
    for (int i = 0; i < 256; ++i) v.append(static_cast<char>(i));
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QByteArray out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtByteArray, KnownBinaryPayload) { check(QByteArray("\x01\x02\x03\x7f", 4)); }

TEST(QtByteArray, Large)
{
    QByteArray v(2000, '\xAB');
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QByteArray out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

// ---- QVector ----

TEST(QtVector, Empty)
{
    QVector<int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QVector<int> out{99};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtVector, Ints)
{
    QVector<int> v{1, 2, 3, 4, 5};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QVector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtVector, Doubles)
{
    QVector<double> v{1.1, 2.2, 3.3};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QVector<double> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtVector, Strings)
{
    QVector<QString> v{QString("a"), QString("bb"), QString("ccc")};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QVector<QString> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtVector, Nested)
{
    QVector<QVector<int>> v;
    v.append({1, 2});
    v.append({3, 4, 5});
    v.append(QVector<int>());
    check(v);
}

TEST(QtVector, Large)
{
    QVector<int> v;
    for (int i = 0; i < 2000; ++i) v.append(i * 7);
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QVector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtVector, JsonRoundtrip)
{
    QVector<int> v{10, 20, 30};
    auto json = Serio::serialize<Serio::JSON>({}, v);
    QVector<int> out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_EQ(out, v);
}

// ---- QList ----

TEST(QtList, Empty)
{
    QList<int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QList<int> out{1, 2};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtList, Ints)
{
    QList<int> v{5, 4, 3, 2, 1};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QList<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtList, Strings)
{
    QList<QString> v{QString("foo"), QString("bar"), QString("baz")};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QList<QString> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtList, Bool) { check(QList<bool>{true, false, true}); }

TEST(QtList, JsonRoundtrip)
{
    QList<int> v{1, 2, 3};
    auto json = Serio::serialize<Serio::JSON>({}, v);
    QList<int> out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_EQ(out, v);
}

// ---- QQueue ----

TEST(QtQueue, Empty)
{
    QQueue<int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QQueue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtQueue, Ints)
{
    QQueue<int> v;
    v.enqueue(10);
    v.enqueue(20);
    v.enqueue(30);
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QQueue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtQueue, Strings)
{
    QQueue<QString> v;
    v.enqueue(QString("first"));
    v.enqueue(QString("second"));
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QQueue<QString> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

// ---- QStack ----

TEST(QtStack, Empty)
{
    QStack<int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QStack<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtStack, Ints)
{
    QStack<int> v;
    v.push(1);
    v.push(2);
    v.push(3);
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QStack<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtStack, Strings)
{
    QStack<QString> v;
    v.push(QString("bottom"));
    v.push(QString("top"));
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QStack<QString> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtStack, Double)
{
    QStack<double> s;
    s.push(1.1);
    s.push(2.2);
    check(s);
}

// ---- QSet ----

TEST(QtSet, Empty)
{
    QSet<int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QSet<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtSet, Ints)
{
    QSet<int> v{1, 2, 3, 4, 5};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QSet<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtSet, Strings)
{
    QSet<QString> v{QString("a"), QString("b"), QString("c")};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QSet<QString> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtSet, JsonRoundtrip)
{
    QSet<int> v{10, 20, 30};
    auto json = Serio::serialize<Serio::JSON>({}, v);
    QSet<int> out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_EQ(out, v);
}

// ---- QStringList ----

TEST(QtStringList, Empty)
{
    QStringList v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QStringList out{QString("placeholder")};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtStringList, Simple)
{
    QStringList v{QString("alpha"), QString("beta"), QString("gamma")};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QStringList out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtStringList, Single)
{
    QStringList v{QString("only")};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QStringList out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtStringList, WithEmpty) { check(QStringList{"first", "", "third"}); }

TEST(QtStringList, JsonRoundtrip)
{
    QStringList v{QString("one"), QString("two"), QString("three")};
    auto json = Serio::serialize<Serio::JSON>({}, v);
    QStringList out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_EQ(out, v);
}

// ---- QVarLengthArray ----

TEST(QtVarLengthArray, Empty)
{
    QVarLengthArray<int, 8> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QVarLengthArray<int, 8> out;
    out.append(99);
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.size(), 0);
}

TEST(QtVarLengthArray, WithinInlineCapacity)
{
    QVarLengthArray<int, 8> v{1, 2, 3, 4};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QVarLengthArray<int, 8> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), v.size());
    for (int i = 0; i < v.size(); ++i) EXPECT_EQ(out[i], v[i]);
}

TEST(QtVarLengthArray, ExceedsInlineCapacity)
{
    QVarLengthArray<int, 4> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QVarLengthArray<int, 4> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), v.size());
    for (int i = 0; i < v.size(); ++i) EXPECT_EQ(out[i], v[i]);
}

TEST(QtVarLengthArray, Doubles)
{
    QVarLengthArray<double, 4> v{1.1, 2.2, 3.3};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QVarLengthArray<double, 4> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), v.size());
    for (int i = 0; i < v.size(); ++i) EXPECT_DOUBLE_EQ(out[i], v[i]);
}

// ---- QMap ----

TEST(QtMap, Empty)
{
    QMap<int, int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QMap<int, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtMap, IntToInt)
{
    QMap<int, int> v{{1, 10}, {2, 20}, {3, 30}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QMap<int, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtMap, StringToString)
{
    QMap<QString, QString> v{{QString("a"), QString("1")}, {QString("b"), QString("2")}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QMap<QString, QString> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtMap, IntToString)
{
    QMap<int, QString> v{{1, QString("one")}, {2, QString("two")}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QMap<int, QString> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtMap, StringKeys) { check(QMap<QString, int>{{"a", 1}, {"b", 2}}); }

TEST(QtMap, JsonRoundtrip)
{
    QMap<int, int> v{{1, 100}, {2, 200}};
    auto json = Serio::serialize<Serio::JSON>({}, v);
    QMap<int, int> out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_EQ(out, v);
}

// ---- QHash ----

TEST(QtHash, Empty)
{
    QHash<int, int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QHash<int, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtHash, IntToInt)
{
    QHash<int, int> v{{1, 10}, {2, 20}, {3, 30}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QHash<int, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtHash, StringToInt)
{
    QHash<QString, int> v{{QString("x"), 1}, {QString("y"), 2}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QHash<QString, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

// ---- QMultiMap ----

TEST(QtMultiMap, Empty)
{
    QMultiMap<int, int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QMultiMap<int, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtMultiMap, WithDuplicateKeys)
{
    QMultiMap<int, QString> v;
    v.insert(1, QString("a"));
    v.insert(1, QString("b"));
    v.insert(2, QString("c"));
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QMultiMap<int, QString> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.size(), v.size());
    EXPECT_EQ(out.count(1), 2);
    EXPECT_EQ(out.count(2), 1);
}

TEST(QtMultiMap, SameKey)
{
    QMultiMap<QString, int> v;
    v.insert(QString("key"), 1);
    v.insert(QString("key"), 2);
    v.insert(QString("key"), 3);
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QMultiMap<QString, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.size(), 3);
    EXPECT_EQ(out.count(QString("key")), 3);
}

TEST(QtMultiMap, StringValues)
{
    QMultiMap<int, QString> m;
    m.insert(1, "a");
    m.insert(1, "b");
    check(m);
}

// ---- QMultiHash ----

TEST(QtMultiHash, Empty)
{
    QMultiHash<int, int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QMultiHash<int, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(QtMultiHash, WithDuplicateKeys)
{
    QMultiHash<int, int> v;
    v.insert(1, 100);
    v.insert(1, 200);
    v.insert(2, 300);
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QMultiHash<int, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.size(), v.size());
    EXPECT_EQ(out.count(1), 2);
    EXPECT_EQ(out.count(2), 1);
}

TEST(QtMultiHash, StringKey)
{
    QMultiHash<QString, QString> v;
    v.insert(QString("k"), QString("v1"));
    v.insert(QString("k"), QString("v2"));
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    QMultiHash<QString, QString> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.size(), 2);
    EXPECT_EQ(out.count(QString("k")), 2);
}

#endif  // SERIO_TEST_QT
