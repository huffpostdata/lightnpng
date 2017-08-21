#include "compress.h"

#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include "upstream/miniz.c"

static const size_t MaxChunkSize = 64 * 1024;

CompressResult
compress(uint8_t* bytes, size_t len)
{
  mz_stream stream;
  stream.zalloc = 0;
  stream.zfree = 0;
  stream.opaque = 0;

  CompressResult ret = { COMPRESS_OK, 0, std::list<std::string>() };

  int zerr = mz_deflateInit2(
    &stream,
    MZ_BEST_SPEED,
    MZ_DEFLATED,
    15,           // windowBits: 32kb window
    9,            // memLevel: maximum memory => maximum speed
    MZ_RLE        // because zlib manual says it's fast and good for PNG
  );
  if (zerr != MZ_OK) {
    ret.status = COMPRESS_ENOMEM;
    return ret;
  }

  stream.avail_in = len;
  stream.next_in = bytes;

  uint8_t chunk[MaxChunkSize];  // allocated on the stack: cannot error

  do {
    stream.avail_out = MaxChunkSize;
    stream.next_out = chunk;

    mz_deflate(&stream, MZ_FINISH); // cannot error

    size_t chunkSize = MaxChunkSize - stream.avail_out;
    ret.data.emplace_front(reinterpret_cast<char*>(chunk), chunkSize); // TODO catch out-of-mem here
    ret.nBytes += chunkSize;
  } while (stream.avail_out == 0);
  ret.data.reverse();

  return ret;
}

