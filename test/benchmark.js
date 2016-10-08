'use strict'

// Run "time node test/benchmark.js" before and after attempted optimizations.

const lightnpng = require('../index')
const Canvas = require('canvas')

// Right now we aren't compressing much. In the future, we should flesh out
// more examples.

function compressTest(test) {
  return lightnpng.native_argb32_to_png(
    test.buffer, test.width, test.height, test.stride
  )
}

function buildRectanglesTest() {
  const canvas = new Canvas(640, 480)
  const ctx = canvas.getContext('2d')

  ctx.fillStyle = 'red'
  ctx.fillRect(10, 10, 200, 340)
  ctx.fillStyle = 'green'
  ctx.fillRect(50, 50, 30, 400);

  const cairoBuf = canvas.toBuffer('raw') // ARGB, native-endian data

  return {
    buffer: cairoBuf,
    width: canvas.width,
    height: canvas.height,
    stride: canvas.stride
  }
}

const rectangles = buildRectanglesTest()
const N = 1000

let nBytes = 0;
for (let i = 0; i < N; i++) {
  nBytes += compressTest(rectangles).length
}

console.log(`Compressed ${N} times for a total of ${nBytes} bytes`)
