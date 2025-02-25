#include <gtest/gtest.h>

#include <random>
#include <string>

#include "crypto.h"

// TEST 0 -- emptyString
class Base64Test0 : public testing::Test
{
protected:
  crypto::Base64 b64;
};

TEST_F (Base64Test0, encode) { EXPECT_EQ (b64.encode (""), ""); }

TEST_F (Base64Test0, decode) { EXPECT_EQ (b64.decode (""), ""); }

// TEST 1 -- Stress Test (1 Million Characters)
class Base64Test1 : public testing::Test
{
protected:
  std::string input;
  crypto::Base64 b64;

  void
  SetUp () override
  {
    input = std::string (1000000, 'X');
  }
};

TEST_F (Base64Test1, encodeThenDecode)
{
  auto encoded = b64.encode (input);
  auto decoded = b64.decode (encoded);
  EXPECT_EQ (decoded, input);
}

// TEST 2-- Encode/Decode with Null Character
class Base64Test2 : public testing::Test
{
protected:
  crypto::Base64 b64;
};

TEST_F (Base64Test2, nullCharacterHandling)
{
  std::string input
      = std::string ("Hello\0World", 11); // "Hello" + null + "World"
  auto encoded = b64.encode (input);
  auto decoded = b64.decode (encoded);
  EXPECT_EQ (decoded, input);
}

static std::string
generateRandomString (size_t length)
{
  const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstu"
                            "vwxyz0123456789!@#$%^&*()_+-=[]{}|;':,./<>?";
  std::random_device rd;
  std::mt19937 gen (rd ());
  std::uniform_int_distribution<> dis (0, chars.size () - 1);

  std::string str;
  for (size_t i = 0; i < length; ++i)
    {
      str += chars[dis (gen)];
    }
  return str;
}

// TEST 3 -- RandomString
class Base64Test3 : public testing::Test
{
protected:
  std::string input;
  crypto::Base64 b64;

  void
  SetUp () override
  {
    input = generateRandomString (128);
  }
};

TEST_F (Base64Test3, encodeThenDecode)
{
  auto encoded = b64.encode (input);
  auto decoded = b64.decode (encoded);
  EXPECT_EQ (decoded, input);
}

// TEST 4 -- RandomString
class CryptoTest0 : public testing::Test
{
protected:
  std::string input;
  crypto::Base64 b64;

  void
  SetUp () override
  {
    input = generateRandomString (128);
  }
};

TEST_F (CryptoTest0, VigenereRandomString)
{
  auto encrypted = crypto::encrypt (input);
  auto decrypted = crypto::decrypt (encrypted);
  EXPECT_EQ (input, decrypted);
}

TEST_F (CryptoTest0, WholeProcess)
{
  auto cipher = b64.encode (crypto::encrypt (input));
  auto plain = crypto::decrypt (b64.decode (cipher));
  EXPECT_EQ (input, plain);
}

// TEST 5 -- Stress Test (1 Million Characters)
class CryptoTest1 : public testing::Test
{
protected:
  std::string input;
  crypto::Base64 b64;

  void
  SetUp () override
  {
    input = std::string (1000000, ')');
  }
};

TEST_F (CryptoTest1, VigenereRandomString)
{
  auto encrypted = crypto::encrypt (input);
  auto decrypted = crypto::decrypt (encrypted);
  EXPECT_EQ (input, decrypted);
}

TEST_F (CryptoTest1, WholeProcess)
{
  auto cipher = b64.encode (crypto::encrypt (input));
  auto plain = crypto::decrypt (b64.decode (cipher));
  EXPECT_EQ (input, plain);
}
