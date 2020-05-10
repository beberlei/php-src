--TEST--
<<Deprecated>> attribute
--FILE--
<?php

<<Deprecated>>
function test() {
}

<<Deprecated("use test() instead")>>
function test2() {
}


test();
test2();
--EXPECTF--
Deprecated: Function test() is deprecated in %s

Deprecated: Function test2() is deprecated use test() instead in %s
