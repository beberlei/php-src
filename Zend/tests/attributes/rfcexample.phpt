--TEST--
Attributes: RFC Example
--FILE--
<?php
namespace My\Attributes {

    <<PhpAttribute>>
    class SingleArgument {
        public $argumentValue;

        public function __construct($argumentValue) {
             $this->argumentValue = $argumentValue;
        }
    }
}

namespace {
    use My\Attributes\SingleArgument;

    <<SingleArgument("Hello World")>>
    class Foo {
    }

    $reflectionClass = new \ReflectionClass(Foo::class);
    $attributes = $reflectionClass->getAttributes();

    var_dump($attributes[0]->getName());
    var_dump($attributes[0]->getArguments());
    var_dump($attributes[0]->getAsObject());
}
--EXPECTF--
string(28) "My\Attributes\SingleArgument"
array(1) {
  [0]=>
  string(11) "Hello World"
}
object(My\Attributes\SingleArgument)#3 (1) {
  ["argumentValue"]=>
  string(11) "Hello World"
}
