#pragma once

#include <sstream>
#include <cstring>
#include <string>

// This is the maximum number of 8-byte slots that we will pack into a single
// GenericKey template. You should not instantiate anything with more than this
using namespace std;

template <size_t keySize>
class GenericKey {
 public:
  // This is the actual byte size of the key
  static constexpr size_t key_size_byte = keySize * 8UL;

 public:
  // This is the array we use for storing integers
  unsigned char data[key_size_byte];

 private:
  /*
   * TwoBytesToBigEndian() - Change 2 bytes to big endian
   *
   * This function could be achieved using XCHG instruction; so do not write
   * your own
   *
   * i.e. MOV AX, WORD PTR [data]
   *      XCHG AH, AL
   */
  inline static uint16_t TwoBytesToBigEndian(uint16_t data) {
    return htobe16(data);
  }

  /*
   * FourBytesToBigEndian() - Change 4 bytes to big endian format
   *
   * This function uses BSWAP instruction in one atomic step; do not write
   * your own
   *
   * i.e. MOV EAX, WORD PTR [data]
   *      BSWAP EAX
   */
  inline static uint32_t FourBytesToBigEndian(uint32_t data) {
    return htobe32(data);
  }

  /*
   * EightBytesToBigEndian() - Change 8 bytes to big endian format
   *
   * This function uses BSWAP instruction
   */
  inline static uint64_t EightBytesToBigEndian(uint64_t data) {
    return htobe64(data);
  }

  /*
   * TwoBytesToHostEndian() - Converts back two byte integer to host byte order
   */
  inline static uint16_t TwoBytesToHostEndian(uint16_t data) {
    return be16toh(data);
  }

  /*
   * FourBytesToHostEndian() - Converts back four byte integer to host byte
   * order
   */
  inline static uint32_t FourBytesToHostEndian(uint32_t data) {
    return be32toh(data);
  }

  /*
   * EightBytesToHostEndian() - Converts back eight byte integer to host byte
   * order
   */
  inline static uint64_t EightBytesToHostEndian(uint64_t data) {
    return be64toh(data);
  }

  /*
   * ToBigEndian() - Overloaded version for all kinds of integral data types
   */

  inline static uint8_t ToBigEndian(uint8_t data) { return data; }

  inline static uint8_t ToBigEndian(int8_t data) {
    return static_cast<uint8_t>(data);
  }

  inline static uint16_t ToBigEndian(uint16_t data) {
    return TwoBytesToBigEndian(data);
  }

  inline static uint16_t ToBigEndian(int16_t data) {
    return TwoBytesToBigEndian(static_cast<uint16_t>(data));
  }

  inline static uint32_t ToBigEndian(uint32_t data) {
    return FourBytesToBigEndian(data);
  }

  inline static uint32_t ToBigEndian(int32_t data) {
    return FourBytesToBigEndian(static_cast<uint32_t>(data));
  }

  inline static uint64_t ToBigEndian(uint64_t data) {
    return EightBytesToBigEndian(data);
  }

  inline static uint64_t ToBigEndian(int64_t data) {
    return EightBytesToBigEndian(static_cast<uint64_t>(data));
  }

  /*
   * ToHostEndian() - Converts big endian data to host format
   */

  static inline uint8_t ToHostEndian(uint8_t data) { return data; }

  static inline uint8_t ToHostEndian(int8_t data) {
    return static_cast<uint8_t>(data);
  }

  static inline uint16_t ToHostEndian(uint16_t data) {
    return TwoBytesToHostEndian(data);
  }

  static inline uint16_t ToHostEndian(int16_t data) {
    return TwoBytesToHostEndian(static_cast<uint16_t>(data));
  }

  static inline uint32_t ToHostEndian(uint32_t data) {
    return FourBytesToHostEndian(data);
  }

  static inline uint32_t ToHostEndian(int32_t data) {
    return FourBytesToHostEndian(static_cast<uint32_t>(data));
  }

  static inline uint64_t ToHostEndian(uint64_t data) {
    return EightBytesToHostEndian(data);
  }

  static inline uint64_t ToHostEndian(int64_t data) {
    return EightBytesToHostEndian(static_cast<uint64_t>(data));
  }

  /*
   * SignFlip() - Flips the highest bit of a given integral type
   *
   * This flip is logical, i.e. it happens on the logical highest bit of an
   * integer. The actual position on the address space is related to endianess
   * Therefore this should happen first.
   *
   * It does not matter whether IntType is signed or unsigned because we do
   * not use the sign bit
   */
  template <typename IntType>
  inline static IntType SignFlip(IntType data) {
    // This sets 1 on the MSB of the corresponding type
    // NOTE: Must cast 0x1 to the correct type first
    // otherwise, 0x1 is treated as the signed int type, and after leftshifting
    // if it is extended to larger type then sign extension will be used
    IntType mask = static_cast<IntType>(0x1) << (sizeof(IntType) * 8UL - 1);

    return data ^ mask;
  }

 public:
  /*
   * Constructor
   */
  GenericKey() {
    ZeroOut();

    return;
  }
  GenericKey(int) { ZeroOut(); return; }
  // Copy constructor
  GenericKey(const GenericKey &other) { memcpy(data, other.data, key_size_byte); }
  inline GenericKey &operator=(const GenericKey &other) {
    memcpy(data, other.data, key_size_byte);
    return *this;
  }

  /*
   * ZeroOut() - Sets all bits to zero
   */
  inline void ZeroOut() {
    memset(data, 0x00, key_size_byte);

    return;
  }

  /*
   * GetRawData() - Returns the raw data array
   */
  const unsigned char *GetRawData() const { return data; }

  /*
   * AddInteger() - Adds a new integer into the compact form
   *
   * Note that IntType must be of the following 8 types:
   *   int8_t; int16_t; int32_t; int64_t
   * Otherwise the result is undefined
   */
  template <typename IntType>
  inline void AddInteger(IntType data, size_t offset) {
    IntType sign_flipped = SignFlip<IntType>(data);

    // This function always returns the unsigned type
    // so we must use automatic type inference
    auto big_endian = ToBigEndian(sign_flipped);

    // This will almost always be optimized into single move
    memcpy(this->data + offset, &big_endian, sizeof(IntType));

    return;
  }

  /*
   * AddUnsignedInteger() - Adds an unsigned integer of a certain type
   *
   * Only the following unsigned type should be used:
   *   uint8_t; uint16_t; uint32_t; uint64_t
   */
  template <typename IntType>
  inline void AddUnsignedInteger(IntType data, size_t offset) {
    // This function always returns the unsigned type
    // so we must use automatic type inference
    auto big_endian = ToBigEndian(data);

    // This will almost always be optimized into single move
    memcpy(this->data + offset, &big_endian, sizeof(IntType));

    return;
  }

  /*
   * GetInteger() - Extracts an integer from the given offset
   *
   * This function has the same limitation as stated for AddInteger()
   */
  template <typename IntType>
  inline IntType GetInteger(size_t offset) const {
    const IntType *ptr = reinterpret_cast<const IntType *>(data + offset);

    // This always returns an unsigned number
    auto host_endian = ToHostEndian(*ptr);

    return SignFlip<IntType>(static_cast<IntType>(host_endian));
  }

  /*
   * GetUnsignedInteger() - Extracts an unsigned integer from the given offset
   *
   * The same constraint about IntType applies
   */
  template <typename IntType>
  inline IntType GetUnsignedInteger(size_t offset) {
    const IntType *ptr = reinterpret_cast<IntType *>(data + offset);
    auto host_endian = ToHostEndian(*ptr);
    return static_cast<IntType>(host_endian);
  }

 public:
  inline void setFromString(std::string key) {
    ZeroOut();
    istringstream iss(key);
    string temp;
    size_t i = 0;

    while (getline(iss, temp, ',')) {
        AddUnsignedInteger <uint64_t> (stoul(temp), i * sizeof(uint64_t));
        if (++i >= keySize)
            break;
    }
    return;
  }

  /*
   * Compare() - Compares two IntsType object of the same length
   *
   * This function has the same semantics as memcmp(). Negative result means
   * less than, positive result means greater than, and 0 means equal
   */
  static inline int Compare(const GenericKey<keySize> &a,
                            const GenericKey<keySize> &b) {
    return memcmp(a.data, b.data,
                  GenericKey<keySize>::key_size_byte);
  }

  /*
   * LessThan() - Returns true if first is less than the second
   */
  static inline bool LessThan(const GenericKey<keySize> &a,
                              const GenericKey<keySize> &b) {
    return Compare(a, b) < 0;
  }

  /*
   * Equals() - Returns true if first is equivalent to the second
   */
  static inline bool Equals(const GenericKey<keySize> &a,
                            const GenericKey<keySize> &b) {
    return Compare(a, b) == 0;
  }

  inline bool operator<(const GenericKey<keySize> &other) { return memcmp(this->data, other.data,
                                                                    GenericKey<keySize>::key_size_byte) < 0; }
  inline bool operator>(const GenericKey<keySize> &other) { return memcmp(this->data, other.data,
                                                                    GenericKey<keySize>::key_size_byte) > 0; }
  inline bool operator==(const GenericKey<keySize> &other) { return memcmp(this->data, other.data,
                                                                    GenericKey<keySize>::key_size_byte) == 0; }
  // Derived operators
  inline bool operator!=(const GenericKey<keySize> &other) { return !(*this == other); }
  inline bool operator<=(const GenericKey<keySize> &other) { return !(*this > other); }
  inline bool operator>=(const GenericKey<keySize> &other) { return !(*this < other); }
};

/*
 * class GenericComparator - Compares two compact integer key
 */
template <size_t keySize>
class GenericComparator {
 public:
  GenericComparator() {}
  GenericComparator(const GenericComparator &) {}

  /*
   * operator()() - Returns true if lhs < rhs
   */
  inline bool operator()(const GenericKey<keySize> &lhs,
                         const GenericKey<keySize> &rhs) const {
    return GenericKey<keySize>::LessThan(lhs, rhs);
  }
};

/*
 * class GenericEqualityChecker - Compares whether two integer keys are
 *                                    equivalent
 */
template <size_t keySize>
class GenericEqualityChecker {
 public:
  GenericEqualityChecker(){};
  GenericEqualityChecker(const GenericEqualityChecker &){};

  inline bool operator()(const GenericKey<keySize> &lhs,
                         const GenericKey<keySize> &rhs) const {
    return GenericKey<keySize>::Equals(lhs, rhs);
  }
};

/*
 * class GenericHasher - Hash function for integer key
 *
 * This function assumes the length of the integer key is always multiples
 * of 64 bits (8 byte word).
 */
template <size_t keySize>
class GenericHasher {
public:
  GenericHasher() {}

  inline size_t operator()(const GenericKey<keySize> &lhs) const {
    (void)lhs;
    return 0UL;
  }
};

