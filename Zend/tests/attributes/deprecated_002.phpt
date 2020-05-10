--TEST--
<<Deprecated>> attribute compile errors
--FILE--
<?php

<<Deprecated(1234)>>
function test() {
}
--EXPECTF--
Fatal error: <<Deprecated>>: Argument #1 ($message) must be of type string, int given in %s
