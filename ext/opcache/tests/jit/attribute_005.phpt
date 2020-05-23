--TEST--
Opcache\Jit not boolean
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

<<\Opcache\Jit("true")>>
function test() {
}
--EXPECTF--
Fatal error: <<Opcache\Jit>> first argument $enabled must be a boolean in %s
