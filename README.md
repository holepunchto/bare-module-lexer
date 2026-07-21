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

<!-- bare-refgen:api start -->

## API

### Functions

#### `lex`

```ts
lex(input: string | Buffer, encoding?: BufferEncoding, opts?: object): {
  imports: Import[]
  exports: Export[]
}
```

[source](https://github.com/holepunchto/bare-module-lexer/blob/v1.6.2/index.d.ts#L16)

Lex `input` for import and export statements, returning the detected `imports` and `exports`.

**Parameters**

| Parameter   | Type               | Default | Description                                  |
| ----------- | ------------------ | ------- | -------------------------------------------- |
| `input`     | `string \| Buffer` | —       | The source to lex, as a string or buffer.    |
| `encoding?` | `BufferEncoding`   | —       | The encoding of `input` when it is a string. |
| `opts?`     | `object`           | —       | Reserved; currently unused.                  |

**Throws**

- `TypeError` — `input` is not a string or buffer.

### Types

#### `Import`

```ts
interface Import {
  specifier: string
  type: number
  names: string[]
  attributes: { [attribute: string]: string }
  position: [importStart: number, specifierStart: number, specifierEnd: number]
}
```

[source](https://github.com/holepunchto/bare-module-lexer/blob/v1.6.2/index.d.ts#L3)

An import detected in the source. `type` is a combination of the `lex.constants` flags (e.g. `REQUIRE`, `IMPORT`, `DYNAMIC`); `position` holds the offsets `[importStart, specifierStart, specifierEnd]`.

#### `Export`

```ts
interface Export {
  name: string
  position: [exportStart: number, nameStart: number, nameEnd: number]
}
```

[source](https://github.com/holepunchto/bare-module-lexer/blob/v1.6.2/index.d.ts#L11)

An export detected in the source; `position` holds the offsets `[exportStart, nameStart, nameEnd]`.

<!-- bare-refgen:api end -->

## License

Apache-2.0
