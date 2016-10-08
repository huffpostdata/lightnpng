'use strict'

const lightnpng = require('../index');

const assert = require('assert');
const fs = require('fs');
const Canvas = require('canvas');

describe('lightnpng', () => {
  describe('argb32_to_png', () => {
    describe('in the README example', () => {
      const canvas = new Canvas(640, 480)
      const ctx = canvas.getContext('2d')

      ctx.fillStyle = 'red'
      ctx.fillRect(10, 10, 200, 340)
      ctx.fillStyle = 'green'
      ctx.fillRect(50, 50, 30, 400);

      const cairoBuf = canvas.toBuffer('raw') // ARGB, native-endian data

      it('should render a PNG Buffer', () => {
        const png = lightnpng.native_argb32_to_png(cairoBuf, 640, 480, canvas.stride);
        fs.writeFileSync(`${__dirname}/../t.png`, png)
        assert.ok(Buffer.isBuffer(png));
        assert.deepEqual(png, fs.readFileSync(`${__dirname}/example-from-readme.png`))
      })

      it('should throw when width exceeds stride', () => {
        assert.throws(
          () => { lightnpng.native_argb32_to_png(cairoBuf, 800, 480, canvas.stride) },
          Error
        )
      });

      it('should throw when height*stride exceeds buffer size', () => {
        assert.throws(
          () => { lightnpng.native_argb32_to_png(cairoBuf, 640, 480, canvas.stride + 1) },
          Error
        )
      });

      it('should return zero-length PNG', () => {
        assert.doesNotThrow(() => lightnpng.native_argb32_to_png(cairoBuf, 0, 0, 0))
      });
    });
  });
});
