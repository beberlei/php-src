--TEST--
Attributes can deal with AST nodes.
--FILE--
<?php

define('V1', strtoupper(php_sapi_name()));

<<A1([V1 => V1])>>
class C1
{
	public const BAR = 'bar';
}

$ref = new \ReflectionClass(C1::class);
$attr = $ref->getAttributes();
var_dump(count($attr));

$args = $attr[0]->getArguments();
var_dump(count($args), $args[0][V1] === V1);

echo "\n";

<<A1(V1, 1 + 2, C1::class)>>
class C2 { }

$ref = new \ReflectionClass(C2::class);
$attr = $ref->getAttributes();
var_dump(count($attr));

$args = $attr[0]->getArguments();
var_dump(count($args));
var_dump($args[0] === V1);
var_dump($args[1] === 3);
var_dump($args[2] === C1::class);

echo "\n";

<<A1(self::FOO, C1::BAR)>>
class C3
{
	private const FOO = 'foo';
}

$ref = new \ReflectionClass(C3::class);
$attr = $ref->getAttributes();
var_dump(count($attr));

$args = $attr[0]->getArguments();
var_dump(count($args));
var_dump($args[0] === 'foo');
var_dump($args[1] === C1::BAR);

?>
--EXPECT--
int(1)
int(1)
bool(true)

int(1)
int(3)
bool(true)
bool(true)
bool(true)

int(1)
int(2)
bool(true)
bool(true)
