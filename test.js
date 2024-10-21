const test = require('brittle')
const lex = require('.')

const { REQUIRE, IMPORT, DYNAMIC, ADDON, ASSET, REEXPORT } = lex.constants

test('require(\'id\')', (t) => {
  t.alike(lex('require(\'./foo.js\')'), {
    imports: [{ specifier: './foo.js', type: REQUIRE, position: [0, 9, 17] }],
    exports: []
  })
})

test('require("id")', (t) => {
  t.alike(lex('require("./foo.js")'), {
    imports: [{ specifier: './foo.js', type: REQUIRE, position: [0, 9, 17] }],
    exports: []
  })
})

test('require.addon(\'id\')', (t) => {
  t.alike(lex('require.addon(\'./foo.bare\')'), {
    imports: [{ specifier: './foo.bare', type: REQUIRE | ADDON, position: [0, 15, 25] }],
    exports: []
  })
})

test('require.addon("id")', (t) => {
  t.alike(lex('require.addon("./foo.bare")'), {
    imports: [{ specifier: './foo.bare', type: REQUIRE | ADDON, position: [0, 15, 25] }],
    exports: []
  })
})

test('require.asset(\'id\')', (t) => {
  t.alike(lex('require.asset(\'./foo.txt\')'), {
    imports: [{ specifier: './foo.txt', type: REQUIRE | ASSET, position: [0, 15, 24] }],
    exports: []
  })
})

test('require.asset("id")', (t) => {
  t.alike(lex('require.asset("./foo.txt")'), {
    imports: [{ specifier: './foo.txt', type: REQUIRE | ASSET, position: [0, 15, 24] }],
    exports: []
  })
})

test('require(\'id\', { with: { type: \'name\' } })', (t) => {
  t.alike(lex('require(\'./foo.js\', { with: { type: \'script\' } })'), {
    imports: [{ specifier: './foo.js', type: REQUIRE, position: [0, 9, 17] }],
    exports: []
  })
})

test('require("id", { with: { type: "name" } })', (t) => {
  t.alike(lex('require("./foo.js", { with: { type: "script" } })'), {
    imports: [{ specifier: './foo.js', type: REQUIRE, position: [0, 9, 17] }],
    exports: []
  })
})

test('module.exports = require', (t) => {
  t.alike(lex('module.exports = require(\'./foo.js\')'), {
    imports: [{ specifier: './foo.js', type: REQUIRE | REEXPORT, position: [17, 26, 34] }],
    exports: []
  })
})

test('exports = require', (t) => {
  t.alike(lex('exports = require(\'./foo.js\')'), {
    imports: [{ specifier: './foo.js', type: REQUIRE | REEXPORT, position: [10, 19, 27] }],
    exports: []
  })
})

test('module.exports = exports = require', (t) => {
  t.alike(lex('module.exports = exports = require(\'./foo.js\')'), {
    imports: [{ specifier: './foo.js', type: REQUIRE | REEXPORT, position: [27, 36, 44] }],
    exports: []
  })
})

test('module.exports = require.addon', (t) => {
  t.alike(lex('module.exports = require.addon(\'./foo.bare\')'), {
    imports: [{ specifier: './foo.bare', type: REQUIRE | ADDON | REEXPORT, position: [17, 32, 42] }],
    exports: []
  })
})

test('exports = require.addon', (t) => {
  t.alike(lex('exports = require.addon(\'./foo.bare\')'), {
    imports: [{ specifier: './foo.bare', type: REQUIRE | ADDON | REEXPORT, position: [10, 25, 35] }],
    exports: []
  })
})

test('module.exports = exports = require.addon', (t) => {
  t.alike(lex('module.exports = exports = require.addon(\'./foo.bare\')'), {
    imports: [{ specifier: './foo.bare', type: REQUIRE | ADDON | REEXPORT, position: [27, 42, 52] }],
    exports: []
  })
})

test('module.exports = require.asset', (t) => {
  t.alike(lex('module.exports = require.asset(\'./foo.txt\')'), {
    imports: [{ specifier: './foo.txt', type: REQUIRE | ASSET | REEXPORT, position: [17, 32, 41] }],
    exports: []
  })
})

test('exports = require.asset', (t) => {
  t.alike(lex('exports = require.asset(\'./foo.txt\')'), {
    imports: [{ specifier: './foo.txt', type: REQUIRE | ASSET | REEXPORT, position: [10, 25, 34] }],
    exports: []
  })
})

test('module.exports = exports = require.asset', (t) => {
  t.alike(lex('module.exports = exports = require.asset(\'./foo.txt\')'), {
    imports: [{ specifier: './foo.txt', type: REQUIRE | ASSET | REEXPORT, position: [27, 42, 51] }],
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

test('exports[\'name\']', (t) => {
  t.alike(lex('exports[\'foo\'] = 42'), {
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

test('module.exports[\'name\']', (t) => {
  t.alike(lex('module.exports[\'foo\'] = 42'), {
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

test('import \'id\'', (t) => {
  t.alike(lex('import \'./foo.js\''), {
    imports: [{ specifier: './foo.js', type: IMPORT, position: [0, 8, 16] }],
    exports: []
  })
})

test('import * as from \'id\'', (t) => {
  t.alike(lex('import * as foo from \'./foo.js\''), {
    imports: [{ specifier: './foo.js', type: IMPORT, position: [0, 22, 30] }],
    exports: []
  })
})

test('import default from \'id\'', (t) => {
  t.alike(lex('import foo from \'./foo.js\''), {
    imports: [{ specifier: './foo.js', type: IMPORT, position: [0, 17, 25] }],
    exports: []
  })
})

test('import { name } from \'id\'', (t) => {
  t.alike(lex('import { foo } from \'./foo.js\''), {
    imports: [{ specifier: './foo.js', type: IMPORT, position: [0, 21, 29] }],
    exports: []
  })
})

test('import(\'id\')', (t) => {
  t.alike(lex('import(\'./foo.js\')'), {
    imports: [{ specifier: './foo.js', type: IMPORT | DYNAMIC, position: [0, 8, 16] }],
    exports: []
  })
})

test('/* require(\'id\') */', (t) => {
  t.alike(lex('/* require(\'./foo.js\') */'), {
    imports: [],
    exports: []
  })
})

test('// require(\'id\')', (t) => {
  t.alike(lex('// require(\'./foo.js\')'), {
    imports: [],
    exports: []
  })
})
