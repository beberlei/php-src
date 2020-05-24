--TEST--
Resolve attribute names
--FILE--
<?php
function dump_attributes($attributes) {
    $arr = [];
    foreach ($attributes as $attribute) {
        $arr[] = ['name' => $attribute->getName(), 'args' => $attribute->getArguments()];
    }
    var_dump($arr);
}

namespace Doctrine\ORM\Mapping {
    class Entity {
    }
}

namespace Foo {
    use Doctrine\ORM\Mapping\Entity;
    use Doctrine\ORM\Mapping as ORM;

    <<Entity("imported class")>>
    <<ORM\Entity("imported namespace")>>
    <<\Doctrine\ORM\Mapping\Entity("absolute from namespace")>>
    <<\Entity("import absolute from global")>>
    function foo() {
    }
}

namespace {
    class Entity {}

    dump_attributes((new ReflectionFunction('Foo\foo'))->getAttributes());
}
?>
--EXPECTF--
array(4) {
  [0]=>
  array(2) {
    ["name"]=>
    string(27) "Doctrine\ORM\Mapping\Entity"
    ["args"]=>
    array(1) {
      [0]=>
      string(14) "imported class"
    }
  }
  [1]=>
  array(2) {
    ["name"]=>
    string(27) "Doctrine\ORM\Mapping\Entity"
    ["args"]=>
    array(1) {
      [0]=>
      string(18) "imported namespace"
    }
  }
  [2]=>
  array(2) {
    ["name"]=>
    string(27) "Doctrine\ORM\Mapping\Entity"
    ["args"]=>
    array(1) {
      [0]=>
      string(23) "absolute from namespace"
    }
  }
  [3]=>
  array(2) {
    ["name"]=>
    string(6) "Entity"
    ["args"]=>
    array(1) {
      [0]=>
      string(27) "import absolute from global"
    }
  }
}
