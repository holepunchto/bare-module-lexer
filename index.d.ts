import Buffer, { BufferEncoding } from 'bare-buffer'

/** An import detected in the source. `type` is a combination of the `lex.constants` flags (e.g. `REQUIRE`, `IMPORT`, `DYNAMIC`); `position` holds the offsets `[importStart, specifierStart, specifierEnd]`. */
interface Import {
  specifier: string
  type: number
  names: string[]
  attributes: { [attribute: string]: string }
  position: [importStart: number, specifierStart: number, specifierEnd: number]
}

/** An export detected in the source; `position` holds the offsets `[exportStart, nameStart, nameEnd]`. */
interface Export {
  name: string
  position: [exportStart: number, nameStart: number, nameEnd: number]
}

/**
 * Lex `input` for import and export statements, returning the detected `imports` and `exports`.
 * @param input - The source to lex, as a string or buffer.
 * @param encoding - The encoding of `input` when it is a string.
 * @param opts - Reserved; currently unused.
 * @throws {TypeError} `input` is not a string or buffer.
 */
declare function lex(
  input: string | Buffer,
  encoding?: BufferEncoding,
  opts?: object
): {
  imports: Import[]
  exports: Export[]
}

declare namespace lex {
  export { type Import, type Export }

  export const constants: {
    REQUIRE: number
    IMPORT: number
    DYNAMIC: number
    ADDON: number
    ASSET: number
    RESOLVE: number
    REEXPORT: number
  }
}

export = lex
