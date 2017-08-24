[![Build Status](https://travis-ci.org/huffpostdata/node-lightnpng.svg?branch=master)](https://travis-ci.org/huffpostdata/node-lightnpng)

Converts a Buffer to PNG format -- very, very quickly.

It's pronounced: "lightn--PING!". As in, PING! Your PNG is ready before you can
say lightning.

# Usage

First, install it in your project: `npm install --save node-lightnpng`

Then use it.

```
'use strict'

// For this example, we'll build an ARGB Buffer with node-canvas:
// https://github.com/Automattic/node-canvas
const Canvas = require('canvas')
const canvas = new Canvas(640, 480)
const ctx = canvas.getContext('2d')

ctx.fillStyle = 'red'
ctx.fillRect(10, 10, 200, 340)
ctx.fillStyle = 'green'
ctx.fillRect(50, 50, 30, 400);

const cairoBuf = canvas.toBuffer('raw') // ARGB, native-endian data

// Now's where this library comes in: let's create a PNG!
const lightnpng = require('node-lightnpng')
var png = lightnpng.native_argb32_to_png(
  cairoBuf,
  640,          // width (in px)
  480,          // height (in px)
  canvas.stride // number of bytes per row of image
)
```

# Design philosophy

We start with zero features.

Features that don't inhibit speed are great.

For features that _do_ slow things down, apply the rule: a 5 percent slowdown
should make this library useful to three times as many people as already use it.

## Options

Basically, we allow no options at all ... _except_:

* Custom width, height and row stride
* Input conversion from ARGB32 to RGB24 by dropping the alpha channel

### stride

Some drawing libraries may align their arrays to machine word boundaries.
The `stride` option specifies how many bytes are in each row.

# LICENSE

BSD-style 2-clause license
