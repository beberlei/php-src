<?php

/** @generate-class-entries */

final class Attribute
{
    public int $flags;

    public function __construct(int $flags = Attribute::TARGET_ALL) {}
}

final class ReturnTypeWillChange
{
    public function __construct() {}
}

final class NamedParameterAlias
{
    public string $alias;

    public function __construct(string $alias) {}
}
