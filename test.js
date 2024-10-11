const test = require('brittle')
const lex = require('.')

test('require', (t) => {
  t.alike(lex('require(\'./foo.js\')'), {
    imports: [{ specifier: './foo.js', type: 0, exported: false }],
    exports: []
  })
})

test('require.addon', (t) => {
  t.alike(lex('require.addon(\'./foo.bare\')'), {
    imports: [{ specifier: './foo.bare', type: lex.constants.ADDON, exported: false }],
    exports: []
  })
})

test('require.asset', (t) => {
  t.alike(lex('require.asset(\'./foo.bare\')'), {
    imports: [{ specifier: './foo.bare', type: lex.constants.ASSET, exported: false }],
    exports: []
  })
})

test('require with type', (t) => {
  t.alike(lex('require(\'./foo.js\', { with: { type: \'script\' } })'), {
    imports: [{ specifier: './foo.js', type: 0, exported: false }],
    exports: []
  })
})

test('require module.exports assignment', (t) => {
  t.alike(lex('module.exports = require(\'./foo.js\')'), {
    imports: [{ specifier: './foo.js', type: 0, exported: true }],
    exports: []
  })
})

test('require exports assigment', (t) => {
  t.alike(lex('exports = require(\'./foo.js\')'), {
    imports: [
      { specifier: './foo.js', type: 0, exported: true } // False positive
    ],
    exports: []
  })
})

test('require module.exports and exports assigment', (t) => {
  t.alike(lex('module.exports = exports = require(\'./foo.js\')'), {
    imports: [{ specifier: './foo.js', type: 0, exported: true }],
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

test('module.exports[\'name\']', (t) => {
  t.alike(lex('exports[\'foo\'] = 42'), {
    imports: [],
    exports: [{ name: 'foo' }]
  })
})

test('import', (t) => {
  t.alike(lex('import \'./foo.js\''), {
    imports: [{ specifier: './foo.js', type: 0, exported: false }],
    exports: []
  })
})

test('import * as', (t) => {
  t.alike(lex('import * as foo from \'./foo.js\''), {
    imports: [{ specifier: './foo.js', type: 0, exported: false }],
    exports: []
  })
})

test('import default from', (t) => {
  t.alike(lex('import foo from \'./foo.js\''), {
    imports: [{ specifier: './foo.js', type: 0, exported: false }],
    exports: []
  })
})

test('import named from', (t) => {
  t.alike(lex('import { foo } from \'./foo.js\''), {
    imports: [{ specifier: './foo.js', type: 0, exported: false }],
    exports: []
  })
})
