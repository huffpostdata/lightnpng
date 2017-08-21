#ifndef _COMPRESS_H
#define _COMPRESS_H

#include <list>
#include <string>

typedef enum {
  COMPRESS_OK,
  COMPRESS_ENOMEM
} CompressStatus;

struct CompressResult {
  CompressStatus status;
  size_t nBytes;
  std::list<std::string> data;
};

/**
 * Creates a zlib-compatible set of bytes that is (usually) smaller than the
 * input.
 */
CompressResult compress(uint8_t* buf, size_t bufLen);

#endif /* _COMPRESS_H */
