cmake_minimum_required(VERSION 3.25)

project(bare_module_lexer C)

include(bare)

add_bare_module(bare_module_lexer)

target_sources(
  ${bare_module_lexer}
  PRIVATE
    binding.c
)
