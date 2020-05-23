--TEST--
Opcache\Jit Attribute
--INI--
opcache.enable=1
opcache.enable_cli=1
opcache.file_update_protection=0
opcache.jit_buffer_size=64
opcache.jit=1245
opcache.jit_debug=257
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

<<Opcache\Jit(true)>>
function test() {
    return 1234;
}
function test2() {
}

test2();
test();
?>
--EXPECTF--
JIT$test: ; (%s)%A
