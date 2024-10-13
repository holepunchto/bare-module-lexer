const binding = require('./binding')

module.exports = exports = function lex (input, encoding, opts = {}) {
  if (typeof encoding === 'object' && encoding !== null) {
    opts = encoding
    encoding = null
  }

  return binding.lex(typeof input === 'string' ? Buffer.from(input, encoding) : input)
}

exports.constants = {
  REQUIRE: binding.REQUIRE,
  IMPORT: binding.IMPORT,
  ADDON: binding.ADDON,
  ASSET: binding.ASSET
}
