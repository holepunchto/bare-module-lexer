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

test("require.asset('id', __filename)", (t) => {
  t.alike(lex("require.asset('./foo.txt', __filename)"), {
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

test("require.resolve('id', __filename)", (t) => {
  t.alike(lex("require.resolve('./foo.js', __filename)"), {
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

test("require.asset('id', notTheModule)", (t) => {
  t.alike(lex("require.asset('./foo.txt', notTheModule)"), {
    imports: [],
    exports: []
  })
})

test("require.asset('id', __dirname)", (t) => {
  t.alike(lex("require.asset('./foo.txt', __dirname)"), {
    imports: [],
    exports: []
  })
})

test("require.addon('id', require('./other'))", (t) => {
  t.alike(lex("require.addon('./foo.bare', require('./other'))"), {
    imports: [
      {
        specifier: './other',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [28, 37, 44]
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

test("module.require('id')", (t) => {
  t.alike(lex("module.require('./foo.js')"), {
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
})

test("spread require('id')", (t) => {
  t.alike(lex("f({ ...require('./foo.js') })"), {
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

test('module.exports = { name: value, name }', (t) => {
  t.alike(lex('module.exports = { a: 1, b }'), {
    imports: [],
    exports: [
      { name: 'a', position: [0, 19, 20] },
      { name: 'b', position: [0, 25, 26] }
    ]
  })

  t.alike(lex('module.exports = { a: fn(1, 2), b: [3, 4], c: { d: 5 }, e }'), {
    imports: [],
    exports: [
      { name: 'a', position: [0, 19, 20] },
      { name: 'b', position: [0, 32, 33] },
      { name: 'c', position: [0, 43, 44] },
      { name: 'e', position: [0, 56, 57] }
    ]
  })

  t.alike(
    lex("module.exports = { a: require('./a.js'), b: require('./b.js'), c: require('./c.js') }"),
    {
      imports: [
        {
          specifier: './a.js',
          type: REQUIRE,
          names: [],
          attributes: {},
          position: [22, 31, 37]
        },
        {
          specifier: './b.js',
          type: REQUIRE,
          names: [],
          attributes: {},
          position: [44, 53, 59]
        },
        {
          specifier: './c.js',
          type: REQUIRE,
          names: [],
          attributes: {},
          position: [66, 75, 81]
        }
      ],
      exports: [
        { name: 'a', position: [0, 19, 20] },
        { name: 'b', position: [0, 41, 42] },
        { name: 'c', position: [0, 63, 64] }
      ]
    }
  )
})

test('module.exports = { ...spread, name }', (t) => {
  t.alike(lex('module.exports = { ...spread, a }'), {
    imports: [],
    exports: [{ name: 'a', position: [0, 30, 31] }]
  })
})

test('module.exports = { get name() {} }', (t) => {
  t.alike(lex('module.exports = { get foo() { return 1 } }'), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 23, 26] }]
  })
})

test('module.exports = { get name() { return require(...) } }', (t) => {
  t.alike(lex("module.exports = { get foo() { return require('./a.js') } }"), {
    imports: [
      {
        specifier: './a.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [38, 47, 53]
      }
    ],
    exports: [{ name: 'foo', position: [0, 23, 26] }]
  })

  t.alike(lex("module.exports = { foo, get bar() { return require.addon.resolve('.') } }"), {
    imports: [
      {
        specifier: '.',
        type: REQUIRE | ADDON | RESOLVE,
        names: [],
        attributes: {},
        position: [43, 66, 67]
      }
    ],
    exports: [
      { name: 'foo', position: [0, 19, 22] },
      { name: 'bar', position: [0, 28, 31] }
    ]
  })
})

test('module.exports = { name() { return require(...) } }', (t) => {
  t.alike(lex("module.exports = { baz() { return require('./b.js') } }"), {
    imports: [
      {
        specifier: './b.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [34, 43, 49]
      }
    ],
    exports: [{ name: 'baz', position: [0, 19, 22] }]
  })
})

test('module.exports = { *name() { return require(...) } }', (t) => {
  t.alike(lex("module.exports = { *foo() { return require('./a.js') } }"), {
    imports: [
      {
        specifier: './a.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [35, 44, 50]
      }
    ],
    exports: []
  })
})

test('module.exports = { ...require(...) }', (t) => {
  t.alike(lex("module.exports = { ...require('./a.js') }"), {
    imports: [
      {
        specifier: './a.js',
        type: REQUIRE | REEXPORT,
        names: [],
        attributes: {},
        position: [22, 31, 37]
      }
    ],
    exports: []
  })

  t.alike(lex("module.exports = { foo, ...require('./a.js'), bar }"), {
    imports: [
      {
        specifier: './a.js',
        type: REQUIRE | REEXPORT,
        names: [],
        attributes: {},
        position: [27, 36, 42]
      }
    ],
    exports: [
      { name: 'foo', position: [0, 19, 22] },
      { name: 'bar', position: [0, 46, 49] }
    ]
  })
})

test('module.exports = { "name": require(...) }', (t) => {
  t.alike(lex("module.exports = { 'foo': require('./a.js') }"), {
    imports: [
      {
        specifier: './a.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [26, 35, 41]
      }
    ],
    exports: [{ name: 'foo', position: [0, 20, 23] }]
  })

  t.alike(lex("module.exports = { 'foo-bar': 1, baz }"), {
    imports: [],
    exports: [
      { name: 'foo-bar', position: [0, 20, 27] },
      { name: 'baz', position: [0, 33, 36] }
    ]
  })
})

test('module.exports = { ["name"]: ... }', (t) => {
  t.alike(lex("module.exports = { ['foo-bar']: 1 }"), {
    imports: [],
    exports: [{ name: 'foo-bar', position: [0, 21, 28] }]
  })
})

test('module.exports = { "name"() {} }', (t) => {
  t.alike(lex("module.exports = { 'foo'() { return require('./a.js') } }"), {
    imports: [
      {
        specifier: './a.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [36, 45, 51]
      }
    ],
    exports: [{ name: 'foo', position: [0, 20, 23] }]
  })
})

test('module.exports = { [computed]: require(...) }', (t) => {
  t.alike(lex("module.exports = { [foo]: require('./a.js') }"), {
    imports: [
      {
        specifier: './a.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [26, 35, 41]
      }
    ],
    exports: []
  })

  t.alike(lex("module.exports = { [require('./k.js')]: 1 }"), {
    imports: [
      {
        specifier: './k.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [20, 29, 35]
      }
    ],
    exports: []
  })
})

test('module.exports = { unicode name }', (t) => {
  t.alike(lex('module.exports = { caf\u00e9 }'), {
    imports: [],
    exports: [{ name: 'caf\u00e9', position: [0, 19, 24] }]
  })
})

test('export const name', (t) => {
  t.alike(lex('export const foo = 42'), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 13, 16] }]
  })
})

test('export let name', (t) => {
  t.alike(lex('export let foo = 42'), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 11, 14] }]
  })
})

test('export var name', (t) => {
  t.alike(lex('export var foo = 42'), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 11, 14] }]
  })
})

test('export const name, name', (t) => {
  t.alike(lex('export const a = 1, b = 2'), {
    imports: [],
    exports: [
      { name: 'a', position: [0, 13, 14] },
      { name: 'b', position: [0, 20, 21] }
    ]
  })

  t.alike(lex('export let a, b'), {
    imports: [],
    exports: [
      { name: 'a', position: [0, 11, 12] },
      { name: 'b', position: [0, 14, 15] }
    ]
  })

  t.alike(lex('export const a = fn(1, 2), b = [3, 4]'), {
    imports: [],
    exports: [
      { name: 'a', position: [0, 13, 14] },
      { name: 'b', position: [0, 27, 28] }
    ]
  })
})

test('export const name = require', (t) => {
  t.alike(lex("export const a = require('./foo.js'), b = 2"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [17, 26, 34]
      }
    ],
    exports: [
      { name: 'a', position: [0, 13, 14] },
      { name: 'b', position: [0, 38, 39] }
    ]
  })

  t.alike(lex("export const a = 1\nconst x = require('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [29, 38, 46]
      }
    ],
    exports: [{ name: 'a', position: [0, 13, 14] }]
  })
})

test('export const declarator list and ASI', (t) => {
  t.alike(lex('export const a = 1\nf = g, h = j'), {
    imports: [],
    exports: [{ name: 'a', position: [0, 13, 14] }]
  })

  t.alike(lex('export const a = 1\n  , b = 2'), {
    imports: [],
    exports: [
      { name: 'a', position: [0, 13, 14] },
      { name: 'b', position: [0, 23, 24] }
    ]
  })

  t.alike(lex('export const a = fn(\n  1,\n  2\n), b = 3'), {
    imports: [],
    exports: [
      { name: 'a', position: [0, 13, 14] },
      { name: 'b', position: [0, 33, 34] }
    ]
  })
})

test('export const { pattern }', (t) => {
  t.alike(lex('export const { a, b } = obj'), {
    imports: [],
    exports: [
      { name: 'a', position: [0, 15, 16] },
      { name: 'b', position: [0, 18, 19] }
    ]
  })

  t.alike(lex('export const { a: x } = obj'), {
    imports: [],
    exports: [{ name: 'x', position: [0, 18, 19] }]
  })

  t.alike(lex('export const { a = 1 } = obj'), {
    imports: [],
    exports: [{ name: 'a', position: [0, 15, 16] }]
  })

  t.alike(lex('export const { a, ...rest } = obj'), {
    imports: [],
    exports: [
      { name: 'a', position: [0, 15, 16] },
      { name: 'rest', position: [0, 21, 25] }
    ]
  })

  t.alike(lex('export const { a: { b } } = obj'), {
    imports: [],
    exports: [{ name: 'b', position: [0, 20, 21] }]
  })
})

test('export const [ pattern ]', (t) => {
  t.alike(lex('export const [a, , b] = arr'), {
    imports: [],
    exports: [
      { name: 'a', position: [0, 14, 15] },
      { name: 'b', position: [0, 19, 20] }
    ]
  })
})

test('export const { pattern } = require', (t) => {
  t.alike(lex('export const { a, b } = require("./foo.js")'), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [24, 33, 41]
      }
    ],
    exports: [
      { name: 'a', position: [0, 15, 16] },
      { name: 'b', position: [0, 18, 19] }
    ]
  })
})

test('export const unicode name', (t) => {
  t.alike(lex('export const caf\u00e9 = 1'), {
    imports: [],
    exports: [{ name: 'caf\u00e9', position: [0, 13, 18] }]
  })
})

test('export function name () {}', (t) => {
  t.alike(lex('export function foo () {}'), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 16, 19] }]
  })
})

test('export async function name', (t) => {
  t.alike(lex('export async function foo() {}'), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 22, 25] }]
  })
})

test('export async function* name', (t) => {
  t.alike(lex('export async function* gen() {}'), {
    imports: [],
    exports: [{ name: 'gen', position: [0, 23, 26] }]
  })
})

test('export class Name {}', (t) => {
  t.alike(lex('export class Foo {}'), {
    imports: [],
    exports: [{ name: 'Foo', position: [0, 13, 16] }]
  })
})

test('export default value', (t) => {
  t.alike(lex('export default 42'), {
    imports: [],
    exports: [{ name: 'default', position: [0, 7, 14] }]
  })
})

test('export { name }', (t) => {
  t.alike(lex('export { foo }'), {
    imports: [],
    exports: [{ name: 'foo', position: [0, 9, 12] }]
  })
})

test('export { name, name }', (t) => {
  t.alike(lex('export { foo, bar }'), {
    imports: [],
    exports: [
      { name: 'foo', position: [0, 9, 12] },
      { name: 'bar', position: [0, 14, 17] }
    ]
  })
})

test('export { name as name }', (t) => {
  t.alike(lex('export { foo as bar }'), {
    imports: [],
    exports: [{ name: 'bar', position: [0, 16, 19] }]
  })
})

test('export { name as "string name" }', (t) => {
  t.alike(lex('export { x as "str" }'), {
    imports: [],
    exports: [{ name: 'str', position: [0, 15, 18] }]
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
        names: ['foo'],
        attributes: {},
        position: [0, 21, 29]
      }
    ],
    exports: []
  })
})

test("import { name, name as name } from 'id'", (t) => {
  t.alike(lex("import { foo, bar as baz } from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT,
        names: ['foo', 'bar'],
        attributes: {},
        position: [0, 33, 41]
      }
    ],
    exports: []
  })
})

test('import { "string name" as name } from "id"', (t) => {
  t.alike(lex('import { "str" as y } from "./foo.js"'), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT,
        names: ['str'],
        attributes: {},
        position: [0, 28, 36]
      }
    ],
    exports: []
  })
})

test("import { unicode } from 'id'", (t) => {
  t.alike(lex("import { caf\u00e9 } from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT,
        names: ['caf\u00e9'],
        attributes: {},
        position: [0, 23, 31]
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
        names: ['foo'],
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
          names: ['foo'],
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
        names: ['default', 'bar'],
        attributes: {},
        position: [0, 26, 34]
      }
    ],
    exports: []
  })
})

test("import default, { name, name as name } from 'id'", (t) => {
  t.alike(lex("import foo, { bar, baz as qux } from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT,
        names: ['default', 'bar', 'baz'],
        attributes: {},
        position: [0, 38, 46]
      }
    ],
    exports: []
  })
})

test("import default, * as namespace from 'id'", (t) => {
  t.alike(lex("import def, * as ns from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT,
        names: ['default', '*'],
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

test("import('id', { with: { type: 'name' } })", (t) => {
  t.alike(lex("import('./foo.js', { with: { type: 'json' } })"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT | DYNAMIC,
        names: [],
        attributes: { type: 'json' },
        position: [0, 8, 16]
      }
    ],
    exports: []
  })

  t.alike(lex('import("./foo.js", { with: { type: "json", imports: "./imports.json" } })'), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT | DYNAMIC,
        names: [],
        attributes: { type: 'json', imports: './imports.json' },
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

test("import.meta.addon('id', import.meta.url)", (t) => {
  t.alike(lex("import.meta.addon('./foo.bare', import.meta.url)"), {
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

test('import.meta.addon("id", import.meta.url)', (t) => {
  t.alike(lex('import.meta.addon("./foo.bare", import.meta.url)'), {
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

test("import.meta.addon('id', import.meta.filename)", (t) => {
  t.alike(lex("import.meta.addon('./foo.bare', import.meta.filename)"), {
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

test("import.meta.addon('id', notTheModule)", (t) => {
  t.alike(lex("import.meta.addon('./foo.bare', notTheModule)"), {
    imports: [],
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
        names: ['foo'],
        attributes: {},
        position: [0, 21, 29]
      }
    ],
    exports: []
  })
})

test("export { name, name as name } from 'id'", (t) => {
  t.alike(lex("export { foo, bar as baz } from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT | REEXPORT,
        names: ['foo', 'bar'],
        attributes: {},
        position: [0, 33, 41]
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

test("module.__exportStar(require('id'))", (t) => {
  t.alike(lex('tslib_1.__exportStar(require("./foo.js"), exports)'), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE | REEXPORT,
        names: [],
        attributes: {},
        position: [21, 30, 38]
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

test('comments between tokens', (t) => {
  t.alike(lex("require/* c */('./foo.js')"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [0, 16, 24]
      }
    ],
    exports: []
  })

  t.alike(lex("import def /* c */ from './foo.js'"), {
    imports: [
      {
        specifier: './foo.js',
        type: IMPORT,
        names: ['default'],
        attributes: {},
        position: [0, 25, 33]
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

test('escaped quote in specifier', (t) => {
  t.alike(lex("require('a\\'b')"), {
    imports: [
      {
        specifier: "a\\'b",
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [0, 9, 13]
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

test('regular expression content is inert', (t) => {
  t.alike(lex("const re = /require\\('x'\\)/"), {
    imports: [],
    exports: []
  })

  t.alike(lex("function f() { return /require\\('x'\\)/ }"), {
    imports: [],
    exports: []
  })
})

test('division does not swallow require()', (t) => {
  t.alike(lex("const half = w/2, x = require('./foo.js')/2"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [22, 31, 39]
      }
    ],
    exports: []
  })
})

test('require() inside template substitution', (t) => {
  t.alike(lex('const s = `${require("./foo.js")}`'), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [13, 22, 30]
      }
    ],
    exports: []
  })
})

test('require() inside nested template substitution', (t) => {
  t.alike(lex("`a${`b${require('./foo.js')}`}`"), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [8, 17, 25]
      }
    ],
    exports: []
  })
})

test('template content is inert', (t) => {
  t.alike(lex('const s = `require("./foo.js")`'), {
    imports: [],
    exports: []
  })
})

test('braces inside template substitution', (t) => {
  t.alike(lex('const s = `${ { a: 1 } }`\nrequire("./foo.js")'), {
    imports: [
      {
        specifier: './foo.js',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [26, 35, 43]
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

test('concatenated specifier is not require()', (t) => {
  t.alike(lex("require('./' + n)"), {
    imports: [],
    exports: []
  })

  t.alike(lex('require("./" + n)'), {
    imports: [],
    exports: []
  })

  t.alike(lex("require('./foo/' + n + '.js')"), {
    imports: [],
    exports: []
  })

  t.alike(lex("require.resolve('./' + n)"), {
    imports: [],
    exports: []
  })

  t.alike(lex("require.addon('./' + n)"), {
    imports: [],
    exports: []
  })

  t.alike(lex("require.asset('./' + n)"), {
    imports: [],
    exports: []
  })
})

test('concatenated specifier is not import()', (t) => {
  t.alike(lex("import('./' + n)"), {
    imports: [],
    exports: []
  })

  t.alike(lex('import("./" + n)'), {
    imports: [],
    exports: []
  })

  t.alike(lex("import('./foo/' + n + '.js')"), {
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

test('member access is not require()', (t) => {
  t.alike(lex("obj.require('./foo.js')"), {
    imports: [],
    exports: []
  })

  t.alike(lex("obj?.require('./foo.js')"), {
    imports: [],
    exports: []
  })
})

test('identifier containing keyword is not require()', (t) => {
  t.alike(lex("_require('./foo.js')"), {
    imports: [],
    exports: []
  })

  t.alike(lex("myrequire('./foo.js')"), {
    imports: [],
    exports: []
  })

  t.alike(lex("requires('./foo.js')"), {
    imports: [],
    exports: []
  })
})

test('identifier containing keyword is not exports', (t) => {
  t.alike(lex('myexports.foo = 1'), {
    imports: [],
    exports: []
  })
})

test('import type default from "id"', (t) => {
  t.alike(lex("import type Foo from './foo'"), {
    imports: [],
    exports: []
  })
})

test('import type { name } from "id"', (t) => {
  t.alike(lex("import type { Foo } from './foo'"), {
    imports: [],
    exports: []
  })
})

test('import type { name, name } from "id"', (t) => {
  t.alike(lex("import type { Foo, Bar } from './foo'"), {
    imports: [],
    exports: []
  })
})

test('import type * as namespace from "id"', (t) => {
  t.alike(lex("import type * as Foo from './foo'"), {
    imports: [],
    exports: []
  })
})

test('import type { name as name } from "id"', (t) => {
  t.alike(lex("import type { Foo as Bar } from './foo'"), {
    imports: [],
    exports: []
  })
})

test('export type { name } from "id"', (t) => {
  t.alike(lex("export type { Foo } from './foo'"), {
    imports: [],
    exports: []
  })
})

test('export type { name }', (t) => {
  t.alike(lex('export type { Foo }'), {
    imports: [],
    exports: []
  })
})

test('export type * from "id"', (t) => {
  t.alike(lex("export type * from './foo'"), {
    imports: [],
    exports: []
  })
})

test('export type * as namespace from "id"', (t) => {
  t.alike(lex("export type * as ns from './foo'"), {
    imports: [],
    exports: []
  })
})

test('import { type name } from "id"', (t) => {
  t.alike(lex("import { type Foo } from './foo'"), {
    imports: [],
    exports: []
  })
})

test('import { type name, name } from "id"', (t) => {
  t.alike(lex("import { type Foo, Bar } from './foo'"), {
    imports: [
      {
        specifier: './foo',
        type: IMPORT,
        names: ['Bar'],
        attributes: {},
        position: [0, 31, 36]
      }
    ],
    exports: []
  })
})

test('import { type name as name, name } from "id"', (t) => {
  t.alike(lex("import { type Foo as X, Bar } from './foo'"), {
    imports: [
      {
        specifier: './foo',
        type: IMPORT,
        names: ['Bar'],
        attributes: {},
        position: [0, 36, 41]
      }
    ],
    exports: []
  })
})

test('import default, { type name, name } from "id"', (t) => {
  t.alike(lex("import Foo, { type Bar, Baz } from './foo'"), {
    imports: [
      {
        specifier: './foo',
        type: IMPORT,
        names: ['default', 'Baz'],
        attributes: {},
        position: [0, 36, 41]
      }
    ],
    exports: []
  })
})

test('export { type name } from "id"', (t) => {
  t.alike(lex("export { type Foo } from './foo'"), {
    imports: [],
    exports: []
  })
})

test('export { type name, name } from "id"', (t) => {
  t.alike(lex("export { type Foo, Bar } from './foo'"), {
    imports: [
      {
        specifier: './foo',
        type: IMPORT | REEXPORT,
        names: ['Bar'],
        attributes: {},
        position: [0, 31, 36]
      }
    ],
    exports: []
  })
})

test('export { name, type name } from "id"', (t) => {
  t.alike(lex("export { Foo, type Bar } from './foo'"), {
    imports: [
      {
        specifier: './foo',
        type: IMPORT | REEXPORT,
        names: ['Foo'],
        attributes: {},
        position: [0, 31, 36]
      }
    ],
    exports: []
  })
})

test('export { type name }', (t) => {
  t.alike(lex('export { type Foo }'), {
    imports: [],
    exports: []
  })
})

test('export { type name, name }', (t) => {
  t.alike(lex('export { type Foo, Bar }'), {
    imports: [],
    exports: [{ name: 'Bar', position: [0, 19, 22] }]
  })
})

test("import name = require('id')", (t) => {
  t.alike(lex("import foo = require('./foo')"), {
    imports: [
      {
        specifier: './foo',
        type: REQUIRE,
        names: [],
        attributes: {},
        position: [13, 22, 27]
      }
    ],
    exports: []
  })
})

test("export import name = require('id')", (t) => {
  t.alike(lex("export import foo = require('./foo')"), {
    imports: [
      {
        specifier: './foo',
        type: REQUIRE | REEXPORT,
        names: [],
        attributes: {},
        position: [20, 29, 34]
      }
    ],
    exports: []
  })
})

test('import name = namespace.member', (t) => {
  t.alike(lex('import foo = Bar.Baz'), {
    imports: [],
    exports: []
  })
})

test('export = value', (t) => {
  t.alike(lex('export = foo'), {
    imports: [],
    exports: []
  })
})

test("export = require('id')", (t) => {
  t.alike(lex("export = require('./foo')"), {
    imports: [
      {
        specifier: './foo',
        type: REQUIRE | REEXPORT,
        names: [],
        attributes: {},
        position: [9, 18, 23]
      }
    ],
    exports: []
  })
})

test('type annotation: import("id").name', (t) => {
  t.alike(lex("let x: import('./foo').Bar"), {
    imports: [],
    exports: []
  })
})

test('type alias: typeof import("id")', (t) => {
  t.alike(lex("type X = typeof import('./foo')"), {
    imports: [],
    exports: []
  })
})

test('type alias: import("id").name', (t) => {
  t.alike(lex("type X = import('./foo').Y"), {
    imports: [],
    exports: []
  })
})
