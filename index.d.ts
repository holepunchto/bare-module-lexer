import Buffer, { BufferEncoding } from 'bare-buffer'

declare function lex(
  input: string | Buffer,
  encoding?: BufferEncoding
): {
  imports: {
    specifier: string
    type: number
    names: string[]
    positions: [
      importStart: number,
      specifierStart: number,
      specifierEnd: number
    ]
  }[]

  exports: {
    name: string
    position: [exportStart: number, nameStart: number, nameEnd: number]
  }[]
}

declare namespace lex {
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
