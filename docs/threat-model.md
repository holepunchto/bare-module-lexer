# Threat model

## What this is

`bare-module-lexer` is compiled into Bare. It is listed in `src/builtins.json`, so every Bare process has it. That holds whether or not the process sealed, and no code has to load anything to reach it.

So this addon is part of Bare, and [Bare's threat model](https://github.com/holepunchto/bare/blob/main/docs/threat-model.md) covers it. Read that one first. This one only says where this addon sits in it.

## What it inherits

- **The promise.** Bare promises a sealed process gets no new native code. This addon is native code that is already in, so the seal neither adds it nor takes it away.
- **The attacker.** Untrusted JavaScript in a sealed process. It writes what it likes, runs on as many threads as it wants, and calls anything it can reach in any order and all at once. It can reach all of this addon.
- **The trust.** This addon is trusted, because Bare compiles it in. Whatever you compile in is your security policy, and this is one of the things you picked.
- **The walls.** The same table applies. A thread is not a wall and neither is a realm, so nothing here gets to assume it is alone.
- **The rules.** What Bare says to report, and what Bare says is not a bug, is the same here.

## What counts

- **Counts:** `binding.c` and the JavaScript that ships with it. Sealed JavaScript reaches all of it without loading a thing.
- **Does not count:** tests, benchmarks, and scratch code.

## What this addon adds

Nothing. One pure function. It reads a string and says where the imports and exports are.

It reaches nothing and keeps nothing.

## Where the risk is

It is C walking source that an attacker chose, and the module system runs it on code it may not trust, which Bare names as a risky spot. Memory bugs are all there is here.

The input may be shared memory that another thread writes while it is being read, so a shared input is copied before it is lexed. The lexer and the strings it records then read one input that nothing else can reach.

The lexer is heuristic on purpose, trading correctness for speed. The module system is built to cope with a wrong answer, so a missed import is not a security bug.

## What to report

- Reads outside the input while lexing, on any source at all, including truncated and invalid source
- Allocation or stack growth that an input can drive without bound
- Anything on Bare's report list

Not a bug: a missed or a spurious import or export.
