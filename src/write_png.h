#ifndef _WRITE_PNG_H
#define _WRITE_PNG_H

#include <cstddef>
#include <tr1/cstdint>

typedef enum {
  PNG_OK,
  PNG_ENOMEM
} RawToPngStatus;

/**
 * Converts raw image data to PNG format.
 *
 * Returns OK on success, ENOMEM on error -- not using any memory.
 *
 * Assumes the arguments are consistent -- that way there's no possible error
 * except ENOMEM.
 *
 * Currently only supports native-endian ARGB32 input.
 */
RawToPngStatus
native_argb32_to_png(
  const uint8_t* bytes,
  size_t bytes_len,
  uint8_t** png,
  size_t* png_len,
  size_t width,
  size_t height,
  size_t stride
);

#endif /* _WRITE_PNG_H */
