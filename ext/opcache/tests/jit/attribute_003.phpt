--TEST--
Opcache\Jit on wrong declaration
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

class Foo {
    <<\Opcache\Jit>>
    public $foo;
}
--EXPECTF--
Fatal error: <<Opcache\Jit>> attribute can only be declared on methods or functions in %s
