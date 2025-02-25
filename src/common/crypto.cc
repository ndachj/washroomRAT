#include <string>

#include "crypto.h"

namespace crypto
{
// vigenere encyrpt
std::string
encrypt (const std::string &data)
{
  std::string result;
  size_t key_len = hashedKey.length ();

  for (size_t i = 0; i < data.length (); ++i)
    {
      char encrypted
          = (data[i] + hashedKey[i % key_len]) % 256; // C = (P + K) mod 256
      result += encrypted;
    }

  return result;
}

// vigenere decrypt
std::string
decrypt (const std::string &cipher)
{
  std::string result;
  size_t key_len = hashedKey.length ();

  for (size_t i = 0; i < cipher.length (); ++i)
    {
      char decrypted = (cipher[i] - hashedKey[i % key_len] + 256)
                       % 256; // P = (C - K) mod 256
      result += decrypted;
    }

  return result;
}

// base64encode
std::string
Base64::encode (const std::string &input)
{
  static const char kBase64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

  std::string output;
  size_t inputLen = input.size ();
  int i = 0, j = 0;
  unsigned char a3[3], a4[4];

  for (size_t pos = 0; pos < inputLen; ++pos)
    {
      a3[i++] = input[pos];
      if (i == 3)
        {
          a3ToA4 (a4, a3);
          for (i = 0; i < 4; ++i)
            output += kBase64Alphabet[a4[i]];
          i = 0;
        }
    }

  if (i > 0)
    {
      for (j = i; j < 3; ++j)
        a3[j] = '\0';

      a3ToA4 (a4, a3);
      for (j = 0; j < i + 1; ++j)
        output += kBase64Alphabet[a4[j]];

      while (i++ < 3)
        output += '=';
    }

  return output;
}

// base64decode
std::string
Base64::decode (const std::string &input)
{
  std::string output;
  int i = 0, j = 0;
  unsigned char a3[3], a4[4];

  output.resize (decodedLength (input));
  size_t outIndex = 0;

  for (char c : input)
    {
      if (c == '=')
        break;

      a4[i++] = b64Lookup (c);
      if (i == 4)
        {
          a4ToA3 (a3, a4);
          for (i = 0; i < 3; ++i)
            output[outIndex++] = a3[i];
          i = 0;
        }
    }

  if (i > 0)
    {
      for (j = i; j < 4; ++j)
        a4[j] = 0;

      a4ToA3 (a3, a4);
      for (j = 0; j < i - 1; ++j)
        output[outIndex++] = a3[j];
    }

  output.resize (outIndex); // Adjust to actual size
  return output;
}
} // namespace crypto
