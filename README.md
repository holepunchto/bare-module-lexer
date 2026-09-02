# bare-module-lexer

Heuristic lexer for detecting imports and exports in JavaScript modules. It trades off correctness for performance, aiming to reliably support the most common import and export patterns with as little overhead as possible.

## Usage

```js
const lex = require('bare-module-lexer')

lex(`
  const foo = require('./foo.js')
  exports.bar = 42
`)

// {
//   imports: [
//     { specifier: './foo.js', type: REQUIRE, names: [], position: [ 15, 24, 32 ] }
//   ],
//   exports: [
//     { name: 'bar', position: [ 37, 45, 48 ] }
//   ]
// }
```

## API

See the [`bare-module-lexer` reference](https://docs.pears.com/reference/bare/modules/bare-module-lexer).

## Threat model

`bare-module-lexer` is one of the addons Bare compiles into its binary, so it inherits [Bare's threat model](https://github.com/holepunchto/bare/blob/main/docs/threat-model.md). See [`docs/threat-model.md`](docs/threat-model.md) for where this addon sits in it.

## License

Apache-2.0
