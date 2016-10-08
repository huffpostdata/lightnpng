#include "write_png.h"

#include <nan.h>

/**
 * Takes a Buffer of raw pixel data and outputs a Buffer of PNG data.
 *
 * Throws an Error if we run out of memory when trying to do this. (The memory
 * will all be freed.)
 *
 * Params:
 *   0: input buffer
 *   1: width
 *   2: height
 *   3: stride
 */
NAN_METHOD(NativeArgb32ToPng) {
  if (info.Length() != 4) {
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }

  if (!node::Buffer::HasInstance(info[0])) {
    Nan::ThrowTypeError("First argument must be a Buffer");
    return;
  }

  if (!info[1]->IsUint32() || !info[2]->IsUint32() || !info[3]->IsUint32()) {
    Nan::ThrowTypeError("width, height and stride arguments must be unsigned integers");
    return;
  }

  uint8_t* buf = reinterpret_cast<uint8_t*>(node::Buffer::Data(info[0]));
  size_t   len = node::Buffer::Length(info[0]);
  uint32_t width = info[1]->Uint32Value();
  uint32_t height = info[2]->Uint32Value();
  uint32_t stride = info[3]->Uint32Value();

  // Don't worry about width/height of 0: that makes an empty PNG.
  // Don't worry about width/height being too large: ENOMEM covers that.

  if (stride < width * 4) {
    Nan::ThrowTypeError("the stride must be at least as wide as the number of bytes in a scanline. Your width or stride is wrong.");
    return;
  }

  if (height * stride > len) {
    Nan::ThrowTypeError("the stride and height specify an image larger than the Buffer. Your height and stride are probably wrong.");
    return;
  }

  uint8_t* png;
  size_t pngLen;
  RawToPngStatus status = native_argb32_to_png(buf, len, &png, &pngLen, width, height, stride);
  if (status != PNG_OK) {
    if (status == PNG_ENOMEM) {
      Nan::ThrowTypeError("Error: ran out of memory while converting PNG data");
    } else {
      Nan::ThrowTypeError("Unknown error while converting PNG data");
    }
    return;
  }

  info.GetReturnValue().Set(Nan::NewBuffer(reinterpret_cast<char*>(png), pngLen).ToLocalChecked());
}

void Init(v8::Handle<v8::Object> exports) {
  exports->Set(
      Nan::New("native_argb32_to_png").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(NativeArgb32ToPng)->GetFunction()
  );
}

NODE_MODULE(lightnpng, Init)
