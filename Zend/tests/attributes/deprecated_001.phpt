--TEST--
#[Deprecated] attribute
--FILE--
<?php

error_reporting(E_ALL | E_DEPRECATED);
ini_set('display_errors', true);

#[Deprecated]
function test() {
}

#[Deprecated("use test() instead")]
function test2() {
}

test();
test2();
call_user_func("test");
--EXPECTF--
Deprecated: Function test() is deprecated in %s

Deprecated: Function test2() is deprecated, use test() instead in %s

Deprecated: Function test() is deprecated in %s
