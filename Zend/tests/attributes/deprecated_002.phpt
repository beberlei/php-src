--TEST--
#[Deprecated] attribute
--FILE--
<?php

#[Deprecated("use test() instead")]
function test() {
}

$reflection = new ReflectionFunction('test');
var_dump($reflection->getAttributes()[0]->newInstance());

--EXPECTF--
object(Deprecated)#3 (1) {
  ["message"]=>
  string(18) "use test() instead"
}
