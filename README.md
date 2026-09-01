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

## License

Apache-2.0
