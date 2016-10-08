#include "write_png.h"

#include "crc.h"

#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <forward_list>
#include <memory>
#include <string>
#include <zlib.h>

static const size_t MaxPngChunkSize = 64 * 1024;

static inline void
write_uint32(uint8_t* buf, uint32_t v)
{
  reinterpret_cast<uint32_t*>(buf)[0] = htonl(v);
}

static size_t
write_png_header(uint8_t* png) {
  const uint8_t bytes[] = {
    137, 80, 78, 71, 13, 10, 26, 10
  };
  memcpy(png, bytes, sizeof(bytes));
  return sizeof(bytes);
}

static size_t
write_ihdr(uint8_t* png, int width, int height)
{
  write_uint32(&png[0],  13);         // byte length of IHDR
  write_uint32(&png[4],  0x49484452); // IHDR type
  write_uint32(&png[8],  width);      // 4-byte width
  write_uint32(&png[12], height);     // 4-byte height
  png[16] = 8; // 24-bit pixels
  png[17] = 2; // true color
  png[18] = 0; // deflate compression (only allowed value)
  png[19] = 0; // filter method 0 (only allowed value)
  png[20] = 0; // not interlaced
  write_uint32(&png[21], crc(&png[4], 17));
  return 25;
};

// Holds all zlib data in memory -- even though zlib is designed for streaming
struct ZlibChunks {
  size_t nBytes;
  std::forward_list<std::string> data;
};

static size_t
write_idat(uint8_t* png, const ZlibChunks& chunks)
{
  write_uint32(&png[0],  chunks.nBytes); // byte length of IDAT
  write_uint32(&png[4], 0x49444154);     // IDAT

  uint8_t* w = &png[8];
  for (auto it = chunks.data.cbegin(); it != chunks.data.cend(); ++it) {
    const std::string& chunk = *it;
    memcpy(w, &chunk[0], chunk.length());
    w += chunk.length();
  }

  write_uint32(&png[chunks.nBytes + 8], crc(&png[4], chunks.nBytes + 4));
  return 12 + chunks.nBytes;
}

static size_t
write_iend(uint8_t* png)
{
  const uint8_t bytes[] = {
    0, 0, 0, 0,            // byte length
    73, 69, 78, 68,        // IEND type
    0xae, 0x42, 0x60, 0x82 // CRC (constant)
  };
  memcpy(png, bytes, sizeof(bytes));
  return sizeof(bytes);
}

static void
native_argb32_to_scanlines(
  const uint8_t* argb,
  uint8_t* scanlines,
  size_t width,
  size_t height,
  size_t stride
) {
  uint8_t* w = scanlines;

  for (size_t row = 0; row < height; row++) {
    const uint32_t* argb_row = reinterpret_cast<const uint32_t*>(&argb[row * stride]);

    w[0] = 1; // Filter type 1: "Sub" -- great for swaths of solid color
    ++w;

    uint8_t cr = 0;
    uint8_t cg = 0;
    uint8_t cb = 0;

    for (size_t x = 0; x < width; x++) {
      uint32_t px = argb_row[x];
      uint8_t r = static_cast<uint8_t>(px >> 16);
      uint8_t g = static_cast<uint8_t>(px >> 8);
      uint8_t b = static_cast<uint8_t>(px);

      w[0] = r - cr;
      w[1] = g - cg;
      w[2] = b - cb;

      cr = r;
      cg = g;
      cb = b;

      w += 3;
      // This drops the alpha channel without premultiplying. The effect for
      // semitransparent pixels: they look like they've been painted atop a
      // black background.
    }
  }
}

/**
 * Compresses data into smaller chunks.
 *
 * Returns empty ZlibChunks if there's an error. The only possible error is
 * out-of-memory.
 */
static ZlibChunks
deflate(uint8_t* bytes, size_t len)
{
  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;

  ZlibChunks ret = { 0, std::forward_list<std::string>() };

  int zerr = deflateInit2(
    &stream,
    Z_BEST_SPEED,
    Z_DEFLATED,
    15,           // windowBits: 32kb window
    9,            // memLevel: maximum memory => maximum speed
    Z_RLE         // because zlib manual says it's fast and good for PNG
  );
  if (zerr != Z_OK) {
    return ret; // empty output means error
  }

  stream.avail_in = len;
  stream.next_in = bytes;

  uint8_t chunk[MaxPngChunkSize];  // allocated on the stack: cannot error

  do {
    stream.avail_out = MaxPngChunkSize;
    stream.next_out = chunk;

    deflate(&stream, Z_FINISH); // cannot error

    size_t chunkSize = MaxPngChunkSize - stream.avail_out;
    ret.data.emplace_front(reinterpret_cast<char*>(chunk), chunkSize); // TODO catch out-of-mem here
    ret.nBytes += chunkSize;
  } while (stream.avail_out == 0);
  ret.data.reverse();

  return ret;
}

RawToPngStatus
native_argb32_to_png(
  const uint8_t* bytes,
  size_t bytes_len,
  uint8_t** png,
  size_t* png_len,
  size_t width,
  size_t height,
  size_t stride
) {
  size_t scanlines_len = height * (1 + width * 3);
  std::unique_ptr<uint8_t[]> scanlines(new uint8_t[scanlines_len]);
  native_argb32_to_scanlines(bytes, &scanlines[0], width, height, stride);

  ZlibChunks zlibChunks = deflate(&scanlines[0], scanlines_len);
  if (zlibChunks.nBytes == 0 && scanlines_len > 0) return PNG_ENOMEM;

  *png_len = 57 + zlibChunks.nBytes;
  *png = reinterpret_cast<uint8_t*>(malloc(*png_len));
  if (*png == NULL) return PNG_ENOMEM;

  uint8_t* w = *png;
  w += write_png_header(w);
  w += write_ihdr(w, width, height);
  w += write_idat(w, zlibChunks);
  w += write_iend(w);

  assert(*png + *png_len == w);

  return PNG_OK;
}
