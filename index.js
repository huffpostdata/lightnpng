'use strict'

const methods = require('bindings')('lightnpng.node')

module.exports.native_argb32_to_png = methods.native_argb32_to_png;
