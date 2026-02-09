const test = require('brittle')
const lex = require('.')

const { REQUIRE, IMPORT, DYNAMIC, ADDON, ASSET, REEXPORT, RESOLVE } = lex.constants

test("require('id')", (t) => {
  t.alike(lex("require('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [0, 9, 17]
      }
    ],
    exports: []
  })
})

test('require("id")', (t) => {
  t.alike(lex('require("./foo.js")'), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [0, 9, 17]
      }
    ],
    exports: []
  })
})

test("require.resolve('id')", (t) => {
  t.alike(lex("require.resolve('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE | RESOLVE,
        names: [],
        attributes: {},
        position: [0, 17, 25]
      }
    ],
    exports: []
  })
})

test('require.resolve("id")', (t) => {
  t.alike(lex('require.resolve("./foo.js")'), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE | RESOLVE,
        names: [],
        attributes: {},
        position: [0, 17, 25]
      }
    ],
    exports: []
  })
})

test('require.addon()', (t) => {
  t.alike(lex('require.addon()'), {
    imports: [
      {
        specifier: '',
        type: REQUIRE | ADDON,
        names: [],
        attributes: {},
        position: [0, 14, 14]
      }
    ],
    exports: []
  })
})

test("require.addon('id')", (t) => {
  t.alike(lex("require.addon('./foo.bare')"), {
    imports: [
      {
        specifier: './foo.bare',
        type: REQUIRE | ADDON,
        names: [],
        attributes: {},
        position: [0, 15, 25]
      }
    ],
    exports: []
  })
})

test('require.addon("id")', (t) => {
  t.alike(lex('require.addon("./foo.bare")'), {
    imports: [
      {
        specifier: './foo.bare',
        type: REQUIRE | ADDON,
        names: [],
        attributes: {},
        position: [0, 15, 25]
      }
    ],
    exports: []
  })
})

test("require.addon('id', __filename)", (t) => {
  t.alike(lex("require.addon('./foo.bare', __filename)"), {
    imports: [
      {
        specifier: './foo.bare',
        type: REQUIRE | ADDON,
        names: [],
        attributes: {},
        position: [0, 15, 25]
      }
    ],
    exports: []
  })
})

test('require.addon("id", __filename)', (t) => {
  t.alike(lex('require.addon("./foo.bare", __filename)'), {
    imports: [
      {
        specifier: './foo.bare',
        type: REQUIRE | ADDON,
        names: [],
        attributes: {},
        position: [0, 15, 25]
      }
    ],
    exports: []
  })
})

test('require.addon.resolve()', (t) => {
  t.alike(lex('require.addon.resolve()'), {
    imports: [
      {
        specifier: '',
        type: REQUIRE | ADDON | RESOLVE,
        names: [],
        attributes: {},
        position: [0, 22, 22]
      }
    ],
    exports: []
  })
})

test("require.addon.resolve('id')", (t) => {
  t.alike(lex("require.addon.resolve('./foo.bare')"), {
    imports: [
      {
        specifier: './foo.bare',
        type: REQUIRE | ADDON | RESOLVE,
        names: [],
        attributes: {},
        position: [0, 23, 33]
      }
    ],
    exports: []
  })
})

test('require.addon.resolve("id")', (t) => {
  t.alike(lex('require.addon.resolve("./foo.bare")'), {
    imports: [
      {
        specifier: './foo.bare',
        type: REQUIRE | ADDON | RESOLVE,
        names: [],
        attributes: {},
        position: [0, 23, 33]
      }
    ],
    exports: []
  })
})

test("require.asset('id')", (t) => {
  t.alike(lex("require.asset('./foo.txt')"), {
    imports: [
      {
        specifier: './foo.txt',
        type: REQUIRE | ASSET,
        names: [],
        attributes: {},
        position: [0, 15, 24]
      }
    ],
    exports: []
  })
})

test('require.asset("id")', (t) => {
  t.alike(lex('require.asset("./foo.txt")'), {
    imports: [
      {
        specifier: './foo.txt',
        type: REQUIRE | ASSET,
        names: [],
        attributes: {},
        position: [0, 15, 24]
      }
    ],
    exports: []
  })
})

test("require('id', { with: { type: 'name' } })", (t) => {
  t.alike(lex("require('./foo.js', { with: { type: 'script' } })"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: { type: 'script' },
        position: [0, 9, 17]
      }
    ],
    exports: []
  })

  t.alike(lex("require('./foo.js', { with: { type: 'script', imports: './imports.json' } })"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: { type: 'script', imports: './imports.json' },
        position: [0, 9, 17]
      }
    ],
    exports: []
  })
})

test('require("id", { with: { type: "name" } })', (t) => {
  t.alike(lex('require("./foo.js", { with: { type: "script" } })'), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: { type: 'script' },
        position: [0, 9, 17]
      }
    ],
    exports: []
  })

  t.alike(lex('require("./foo.js", { with: { type: "script", imports: "./imports.json" } })'), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: { type: 'script', imports: './imports.json' },
        position: [0, 9, 17]
      }
    ],
    exports: []
  })
})

test('module.exports = require', (t) => {
  t.alike(lex("module.exports = require('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE | REEXPORT,
        names: [],
        attributes: {},
        position: [17, 26, 34]
      }
    ],
    exports: []
  })
})

test('exports = require', (t) => {
  t.alike(lex("exports = require('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE | REEXPORT,
        names: [],
        attributes: {},
        position: [10, 19, 27]
      }
    ],
    exports: []
  })
})

test('module.exports = exports = require', (t) => {
  t.alike(lex("module.exports = exports = require('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE | REEXPORT,
        names: [],
        attributes: {},
        position: [27, 36, 44]
      }
    ],
    exports: []
  })
})

test('module.exports = require.addon', (t) => {
  t.alike(lex("module.exports = require.addon('./foo.bare')"), {
    imports: [
      {
        specifier: './foo.bare',
        type: REQUIRE | ADDON | REEXPORT,
        names: [],
        attributes: {},
        position: [17, 32, 42]
      }
    ],
    exports: []
  })
})

test('exports = require.addon', (t) => {
  t.alike(lex("exports = require.addon('./foo.bare')"), {
    imports: [
      {
        specifier: './foo.bare',
        type: REQUIRE | ADDON | REEXPORT,
        names: [],
        attributes: {},
        position: [10, 25, 35]
      }
    ],
    exports: []
  })
})

test('module.exports = exports = require.addon', (t) => {
  t.alike(lex("module.exports = exports = require.addon('./foo.bare')"), {
    imports: [
      {
        specifier: './foo.bare',
        type: REQUIRE | ADDON | REEXPORT,
        names: [],
        attributes: {},
        position: [27, 42, 52]
      }
    ],
    exports: []
  })
})

test('module.exports = require.asset', (t) => {
  t.alike(lex("module.exports = require.asset('./foo.txt')"), {
    imports: [
      {
        specifier: './foo.txt',
        type: REQUIRE | ASSET | REEXPORT,
        names: [],
        attributes: {},
        position: [17, 32, 41]
      }
    ],
    exports: []
  })
})

test('exports = require.asset', (t) => {
  t.alike(lex("exports = require.asset('./foo.txt')"), {
    imports: [
      {
        specifier: './foo.txt',
        type: REQUIRE | ASSET | REEXPORT,
        names: [],
        attributes: {},
        position: [10, 25, 34]
      }
    ],
    exports: []
  })
})

test('module.exports = exports = require.asset', (t) => {
  t.alike(lex("module.exports = exports = require.asset('./foo.txt')"), {
    imports: [
      {
        specifier: './foo.txt',
        type: REQUIRE | ASSET | REEXPORT,
        names: [],
        attributes: {},
        position: [27, 42, 51]
      }
    ],
    exports: []
  })
})

test('exports.name', (t) => {
  t.alike(lex('exports.foo = 42'), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 8, 11] }]
  })
})

test('module.exports.name', (t) => {
  t.alike(lex('module.exports.foo = 42'), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 15, 18] }]
  })
})

test("exports['name']", (t) => {
  t.alike(lex("exports['foo'] = 42"), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 9, 12] }]
  })
})

test('exports["name"]', (t) => {
  t.alike(lex('exports["foo"] = 42'), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 9, 12] }]
  })
})

test("module.exports['name']", (t) => {
  t.alike(lex("module.exports['foo'] = 42"), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 16, 19] }]
  })
})

test('module.exports["name"]', (t) => {
  t.alike(lex('module.exports["foo"] = 42'), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 16, 19] }]
  })
})

test('module.exports = {}', (t) => {
  t.alike(lex('module.exports = {}'), {
    imports: [],
    exports: []
  })
})

test('module.exports = { name }', (t) => {
  t.alike(lex('module.exports = { foo }'), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 19, 22] }]
  })
})

test('module.exports = { name, name }', (t) => {
  t.alike(lex('module.exports = { foo, bar }'), {
    imports: [],
    exports: [
      { name: 'foo', position: [0, 19, 22] },
      { name: 'bar', position: [0, 24, 27] }
    ]
  })

  t.alike(lex('module.exports = { foo,\n bar }'), {
    imports: [],
    exports: [
      { name: 'foo', position: [0, 19, 22] },
      { name: 'bar', position: [0, 25, 28] }
    ]
  })
})

test("import 'id'", (t) => {
  t.alike(lex("import './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT,
        names: [],
        attributes: {},
        position: [0, 8, 16]
      }
    ],
    exports: []
  })
})

test("import * as from 'id'", (t) => {
  t.alike(lex("import * as foo from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT,
        names: ['*'],
        attributes: {},
        position: [0, 22, 30]
      }
    ],
    exports: []
  })
})

test("import default from 'id'", (t) => {
  t.alike(lex("import foo from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT,
        names: ['default'],
        attributes: {},
        position: [0, 17, 25]
      }
    ],
    exports: []
  })
})

test("import { name } from 'id'", (t) => {
  t.alike(lex("import { foo } from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT,
        names: [],
        attributes: {},
        position: [0, 21, 29]
      }
    ],
    exports: []
  })
})

test("import { name } from 'id' with { type: 'name' }", (t) => {
  t.alike(lex("import { foo } from './foo.js' with { type: 'script' }"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT,
        names: [],
        attributes: { type: 'script' },
        position: [0, 21, 29]
      }
    ],
    exports: []
  })

  t.alike(
    lex("import { foo } from './foo.js' with { type: 'script', imports: './imports.json' }"),
    {
      imports: [
        {
          specifier: './foo.js',
          type: IMPORT,
          names: [],
          attributes: { type: 'script', imports: './imports.json' },
          position: [0, 21, 29]
        }
      ],
      exports: []
    }
  )
})

test("import default, { name } from 'id'", (t) => {
  t.alike(lex("import foo, { bar } from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT,
        names: ['default'],
        attributes: {},
        position: [0, 26, 34]
      }
    ],
    exports: []
  })
})

test("import('id')", (t) => {
  t.alike(lex("import('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT | DYNAMIC,
        names: [],
        attributes: {},
        position: [0, 8, 16]
      }
    ],
    exports: []
  })
})

test("import.meta.resolve('id')", (t) => {
  t.alike(lex("import.meta.resolve('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT | RESOLVE,
        names: [],
        attributes: {},
        position: [0, 21, 29]
      }
    ],
    exports: []
  })
})

test('import.meta.resolve("id")', (t) => {
  t.alike(lex('import.meta.resolve("./foo.js")'), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT | RESOLVE,
        names: [],
        attributes: {},
        position: [0, 21, 29]
      }
    ],
    exports: []
  })
})
test('import.meta.addon()', (t) => {
  t.alike(lex('import.meta.addon()'), {
    imports: [
      {
        specifier: '',
        type: IMPORT | ADDON,
        names: [],
        attributes: {},
        position: [0, 18, 18]
      }
    ],
    exports: []
  })
})

test("import.meta.addon('id')", (t) => {
  t.alike(lex("import.meta.addon('./foo.bare')"), {
    imports: [
      {
        specifier: './foo.bare',
        type: IMPORT | ADDON,
        names: [],
        attributes: {},
        position: [0, 19, 29]
      }
    ],
    exports: []
  })
})

test('import.meta.addon("id")', (t) => {
  t.alike(lex('import.meta.addon("./foo.bare")'), {
    imports: [
      {
        specifier: './foo.bare',
        type: IMPORT | ADDON,
        names: [],
        attributes: {},
        position: [0, 19, 29]
      }
    ],
    exports: []
  })
})

test("import.meta.addon('id', __filename)", (t) => {
  t.alike(lex("import.meta.addon('./foo.bare', __filename)"), {
    imports: [
      {
        specifier: './foo.bare',
        type: IMPORT | ADDON,
        names: [],
        attributes: {},
        position: [0, 19, 29]
      }
    ],
    exports: []
  })
})

test('import.meta.addon("id", __filename)', (t) => {
  t.alike(lex('import.meta.addon("./foo.bare", __filename)'), {
    imports: [
      {
        specifier: './foo.bare',
        type: IMPORT | ADDON,
        names: [],
        attributes: {},
        position: [0, 19, 29]
      }
    ],
    exports: []
  })
})

test('import.meta.addon.resolve()', (t) => {
  t.alike(lex('import.meta.addon.resolve()'), {
    imports: [
      {
        specifier: '',
        type: IMPORT | ADDON | RESOLVE,
        names: [],
        attributes: {},
        position: [0, 26, 26]
      }
    ],
    exports: []
  })
})

test("import.meta.addon.resolve('id')", (t) => {
  t.alike(lex("import.meta.addon.resolve('./foo.bare')"), {
    imports: [
      {
        specifier: './foo.bare',
        type: IMPORT | ADDON | RESOLVE,
        names: [],
        attributes: {},
        position: [0, 27, 37]
      }
    ],
    exports: []
  })
})

test('import.meta.addon.resolve("id")', (t) => {
  t.alike(lex('import.meta.addon.resolve("./foo.bare")'), {
    imports: [
      {
        specifier: './foo.bare',
        type: IMPORT | ADDON | RESOLVE,
        names: [],
        attributes: {},
        position: [0, 27, 37]
      }
    ],
    exports: []
  })
})

test("import.meta.asset('id')", (t) => {
  t.alike(lex("import.meta.asset('./foo.txt')"), {
    imports: [
      {
        specifier: './foo.txt',
        type: IMPORT | ASSET,
        names: [],
        attributes: {},
        position: [0, 19, 28]
      }
    ],
    exports: []
  })
})

test('import.meta.asset("id")', (t) => {
  t.alike(lex('import.meta.asset("./foo.txt")'), {
    imports: [
      {
        specifier: './foo.txt',
        type: IMPORT | ASSET,
        names: [],
        attributes: {},
        position: [0, 19, 28]
      }
    ],
    exports: []
  })
})

test("export * from 'id'", (t) => {
  t.alike(lex("export * from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT | REEXPORT,
        names: ['*'],
        attributes: {},
        position: [0, 15, 23]
      }
    ],
    exports: []
  })
})

test("export * as from 'id'", (t) => {
  t.alike(lex("export * as foo from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT | REEXPORT,
        names: ['*'],
        attributes: {},
        position: [0, 22, 30]
      }
    ],
    exports: []
  })
})

test("export { name } from 'id'", (t) => {
  t.alike(lex("export { foo } from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT | REEXPORT,
        names: [],
        attributes: {},
        position: [0, 21, 29]
      }
    ],
    exports: []
  })
})

test("__export(require('id'))", (t) => {
  t.alike(lex("__export(require('./foo.js'))"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE | REEXPORT,
        names: [],
        attributes: {},
        position: [9, 18, 26]
      }
    ],
    exports: []
  })
})

test("__exportStar(require('id'))", (t) => {
  t.alike(lex("__exportStar(require('./foo.js'))"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE | REEXPORT,
        names: [],
        attributes: {},
        position: [13, 22, 30]
      }
    ],
    exports: []
  })
})

test("/* require('id') */", (t) => {
  t.alike(lex("/* require('./foo.js') */"), {
    imports: [],
    exports: []
  })

  t.alike(lex("/* require('./foo.js') */ require('./bar.js')"), {
    imports: [
      {
        specifier: './bar.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [26, 35, 43]
      }
    ],
    exports: []
  })

  t.alike(lex("/* require('./foo.js')\nrequire('./bar.js') */ require('./baz.js')"), {
    imports: [
      {
        specifier: './baz.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [46, 55, 63]
      }
    ],
    exports: []
  })
})

test("// require('id')", (t) => {
  t.alike(lex("// require('./foo.js')"), {
    imports: [],
    exports: []
  })

  t.alike(lex("// require('./foo.js')\nrequire('./bar.js')"), {
    imports: [
      {
        specifier: './bar.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [23, 32, 40]
      }
    ],
    exports: []
  })
})

test("'\\\\'; require('id')", (t) => {
  t.alike(lex("'\\\\'; require('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [6, 15, 23]
      }
    ],
    exports: []
  })
})

test("/regex/; require('id')", (t) => {
  t.alike(lex("/'/; require('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [5, 14, 22]
      }
    ],
    exports: []
  })

  t.alike(lex("/'/g; require('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [6, 15, 23]
      }
    ],
    exports: []
  })

  t.alike(lex("/[/]/; require('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [7, 16, 24]
      }
    ],
    exports: []
  })

  t.alike(lex("/\\\\/; require('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [6, 15, 23]
      }
    ],
    exports: []
  })

  t.alike(lex("/[/]/g\nrequire('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [7, 16, 24]
      }
    ],
    exports: []
  })

  t.alike(lex("1 / 2; require('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [7, 16, 24]
      }
    ],
    exports: []
  })

  t.alike(lex("1 / 2 / 3; require('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [11, 20, 28]
      }
    ],
    exports: []
  })
})

test("#!/shebang require('id')", (t) => {
  t.alike(lex("#!/usr/bin/env bare\nrequire('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [20, 29, 37]
      }
    ],
    exports: []
  })
})

test('invalid require', (t) => {
  t.alike(lex("requires('./foo.js')"), {
    imports: [],
    exports: []
  })

  t.alike(lex("'require(\\'./foo.js\\')'"), {
    imports: [],
    exports: []
  })

  t.alike(lex('"require(\'./foo.js\')"'), {
    imports: [],
    exports: []
  })

  t.alike(lex("`require('./foo.js')`"), {
    imports: [],
    exports: []
  })
})

test('invalid import', (t) => {
  t.alike(lex("imported from './foo.js'"), {
    imports: [],
    exports: []
  })

  t.alike(lex("'import' './foo.js'"), {
    imports: [],
    exports: []
  })

  t.alike(lex("'import \\'./foo.js\\''"), {
    imports: [],
    exports: []
  })

  t.alike(lex('"import \'./foo.js\'"'), {
    imports: [],
    exports: []
  })

  t.alike(lex("`import './foo.js'`"), {
    imports: [],
    exports: []
  })

  t.alike(lex(`/[\\\\]'/; 'import "./foo.js"'`), {
    imports: [],
    exports: []
  })
})
