const test = require('brittle')
const lex = require('.')

const { REQUIRE, IMPORT, ADDON, ASSET, REEXPORT } = lex.constants

test('require(\'id\')', (t) => {
  t.alike(lex('require(\'./foo.js\')'), {
    imports: [{ specifier: './foo.js', type: REQUIRE }],
    exports: []
  })
})

test('require("id")', (t) => {
  t.alike(lex('require("./foo.js")'), {
    imports: [{ specifier: './foo.js', type: REQUIRE }],
    exports: []
  })
})

test('require.addon(\'id\')', (t) => {
  t.alike(lex('require.addon(\'./foo.bare\')'), {
    imports: [{ specifier: './foo.bare', type: REQUIRE | ADDON }],
    exports: []
  })
})

test('require.addon("id")', (t) => {
  t.alike(lex('require.addon("./foo.bare")'), {
    imports: [{ specifier: './foo.bare', type: REQUIRE | ADDON }],
    exports: []
  })
})

test('require.asset(\'id\')', (t) => {
  t.alike(lex('require.asset(\'./foo.txt\')'), {
    imports: [{ specifier: './foo.txt', type: REQUIRE | ASSET }],
    exports: []
  })
})

test('require.asset("id")', (t) => {
  t.alike(lex('require.asset("./foo.txt")'), {
    imports: [{ specifier: './foo.txt', type: REQUIRE | ASSET }],
    exports: []
  })
})

test('require(\'id\', { with: { type: \'name\' } })', (t) => {
  t.alike(lex('require(\'./foo.js\', { with: { type: \'script\' } })'), {
    imports: [{ specifier: './foo.js', type: REQUIRE }],
    exports: []
  })
})

test('require("id", { with: { type: "name" } })', (t) => {
  t.alike(lex('require("./foo.js", { with: { type: "script" } })'), {
    imports: [{ specifier: './foo.js', type: REQUIRE }],
    exports: []
  })
})

test('module.exports = require', (t) => {
  t.alike(lex('module.exports = require(\'./foo.js\')'), {
    imports: [{ specifier: './foo.js', type: REQUIRE | REEXPORT }],
    exports: []
  })
})

test('exports = require', (t) => {
  t.alike(lex('exports = require(\'./foo.js\')'), {
    imports: [{ specifier: './foo.js', type: REQUIRE | REEXPORT }],
    exports: []
  })
})

test('module.exports = exports = require', (t) => {
  t.alike(lex('module.exports = exports = require(\'./foo.js\')'), {
    imports: [{ specifier: './foo.js', type: REQUIRE | REEXPORT }],
    exports: []
  })
})

test('module.exports = require.addon', (t) => {
  t.alike(lex('module.exports = require.addon(\'./foo.bare\')'), {
    imports: [{ specifier: './foo.bare', type: REQUIRE | ADDON | REEXPORT }],
    exports: []
  })
})

test('exports = require.addon', (t) => {
  t.alike(lex('exports = require.addon(\'./foo.bare\')'), {
    imports: [{ specifier: './foo.bare', type: REQUIRE | ADDON | REEXPORT }],
    exports: []
  })
})

test('module.exports = exports = require.addon', (t) => {
  t.alike(lex('module.exports = exports = require.addon(\'./foo.bare\')'), {
    imports: [{ specifier: './foo.bare', type: REQUIRE | ADDON | REEXPORT }],
    exports: []
  })
})

test('module.exports = require.asset', (t) => {
  t.alike(lex('module.exports = require.asset(\'./foo.txt\')'), {
    imports: [{ specifier: './foo.txt', type: REQUIRE | ASSET | REEXPORT }],
    exports: []
  })
})

test('exports = require.asset', (t) => {
  t.alike(lex('exports = require.asset(\'./foo.txt\')'), {
    imports: [{ specifier: './foo.txt', type: REQUIRE | ASSET | REEXPORT }],
    exports: []
  })
})

test('module.exports = exports = require.asset', (t) => {
  t.alike(lex('module.exports = exports = require.asset(\'./foo.txt\')'), {
    imports: [{ specifier: './foo.txt', type: REQUIRE | ASSET | REEXPORT }],
    exports: []
  })
})

test('exports.name', (t) => {
  t.alike(lex('exports.foo = 42'), {
    imports: [],
    exports: [{ name: 'foo' }]
  })
})

test('module.exports.name', (t) => {
  t.alike(lex('module.exports.foo = 42'), {
    imports: [],
    exports: [{ name: 'foo' }]
  })
})

test('exports[\'name\']', (t) => {
  t.alike(lex('exports[\'foo\'] = 42'), {
    imports: [],
    exports: [{ name: 'foo' }]
  })
})

test('exports["name"]', (t) => {
  t.alike(lex('exports["foo"] = 42'), {
    imports: [],
    exports: [{ name: 'foo' }]
  })
})

test('module.exports[\'name\']', (t) => {
  t.alike(lex('module.exports[\'foo\'] = 42'), {
    imports: [],
    exports: [{ name: 'foo' }]
  })
})

test('module.exports["name"]', (t) => {
  t.alike(lex('module.exports["foo"] = 42'), {
    imports: [],
    exports: [{ name: 'foo' }]
  })
})

test('import', (t) => {
  t.alike(lex('import \'./foo.js\''), {
    imports: [{ specifier: './foo.js', type: IMPORT }],
    exports: []
  })
})

test('import * as', (t) => {
  t.alike(lex('import * as foo from \'./foo.js\''), {
    imports: [{ specifier: './foo.js', type: IMPORT }],
    exports: []
  })
})

test('import default', (t) => {
  t.alike(lex('import foo from \'./foo.js\''), {
    imports: [{ specifier: './foo.js', type: IMPORT }],
    exports: []
  })
})

test('import named', (t) => {
  t.alike(lex('import { foo } from \'./foo.js\''), {
    imports: [{ specifier: './foo.js', type: IMPORT }],
    exports: []
  })
})
