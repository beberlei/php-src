--TEST--
Opcache\Jit too many arguments
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

<<\Opcache\Jit(false, true)>>
function test() {
}
--EXPECTF--
Fatal error: <<Opcache\Jit>> requires zero or one argument, 2 arguments given in %s
