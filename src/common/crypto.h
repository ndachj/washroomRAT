#ifndef SRC_COMMON_CRYPTO_H
#define SRC_COMMON_CRYPTO_H

#include <string>

namespace crypto
{
static const std::string hashedKey
    = "3aaf87bdc8f86a66aa3f2bf7fda86115f8fcbd70dfb5dc7c367bbd0663c3194e";

// vigenere encyrpt
std::string encrypt (const std::string &data);

// vigenere decrypt
std::string decrypt (const std::string &cipher);

// Base64 encoding and decoding
class Base64
{
public:
  // base64encode
  static std::string encode (const std::string &input);

  // base64decode
  static std::string decode (const std::string &input);

private:
  static constexpr size_t
  decodedLength (const std::string &input)
  {
    size_t padding = 0;
    if (!input.empty () && input.back () == '=')
      {
        padding = (input[input.size () - 2] == '=') ? 2 : 1;
      }
    return ((6 * input.size ()) / 8) - padding;
  }

  static inline void
  a3ToA4 (unsigned char *a4, const unsigned char *a3)
  {
    a4[0] = (a3[0] & 0xfc) >> 2;
    a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
    a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
    a4[3] = (a3[2] & 0x3f);
  }

  static inline void
  a4ToA3 (unsigned char *a3, const unsigned char *a4)
  {
    a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
    a3[1] = ((a4[1] & 0x0f) << 4) + ((a4[2] & 0x3c) >> 2);
    a3[2] = ((a4[2] & 0x03) << 6) + a4[3];
  }

  static inline unsigned char
  b64Lookup (char c)
  {
    if (c >= 'A' && c <= 'Z')
      return c - 'A';
    if (c >= 'a' && c <= 'z')
      return c - 'a' + 26;
    if (c >= '0' && c <= '9')
      return c - '0' + 52;
    if (c == '+')
      return 62;
    if (c == '/')
      return 63;
    return 255;
  }
};

} // namespace crypto

#endif // !SRC_COMMON_CRYPTO_H
