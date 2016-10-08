#include "compress.h"

#include <cstring>
#include <zlib.h>

static const size_t MaxChunkSize = 64 * 1024;

CompressResult
compress(uint8_t* bytes, size_t len)
{
  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;

  CompressResult ret = { COMPRESS_OK, 0, std::forward_list<std::string>() };

  int zerr = deflateInit2(
    &stream,
    Z_BEST_SPEED,
    Z_DEFLATED,
    15,           // windowBits: 32kb window
    9,            // memLevel: maximum memory => maximum speed
    Z_RLE         // because zlib manual says it's fast and good for PNG
  );
  if (zerr != Z_OK) {
    ret.status = COMPRESS_ENOMEM;
    return ret;
  }

  stream.avail_in = len;
  stream.next_in = bytes;

  uint8_t chunk[MaxChunkSize];  // allocated on the stack: cannot error

  do {
    stream.avail_out = MaxChunkSize;
    stream.next_out = chunk;

    deflate(&stream, Z_FINISH); // cannot error

    size_t chunkSize = MaxChunkSize - stream.avail_out;
    ret.data.emplace_front(reinterpret_cast<char*>(chunk), chunkSize); // TODO catch out-of-mem here
    ret.nBytes += chunkSize;
  } while (stream.avail_out == 0);
  ret.data.reverse();

  return ret;
}

