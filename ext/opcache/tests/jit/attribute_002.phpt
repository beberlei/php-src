--TEST--
Opcache\Jit Attribute disables function
--INI--
opcache.enable=1
opcache.enable_cli=1
opcache.file_update_protection=0
opcache.jit_buffer_size=64
opcache.jit=1205
opcache.jit_debug=257
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

<<Opcache\Jit(false)>>
function test() {
    return 1234;
}
function test2() {
}

test();
test2();
?>
--EXPECTF--
JIT$%s: ; (%s)
	sub $0x10, %s
	add $0x10, %s
	mov $ZEND_RETURN_SPEC_CONST_LABEL, %s
	jmp *%s

JIT$test2: ; (%s)%A
