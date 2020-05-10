#include "zend.h"
#include "zend_API.h"
#include "zend_attributes.h"

ZEND_API zend_class_entry *zend_ce_php_attribute;
ZEND_API zend_class_entry *zend_ce_php_compiler_attribute;
ZEND_API zend_class_entry *zend_ce_deprecated_attribute;

static HashTable internal_validators;

void zend_attribute_validate_phpattribute(zend_attribute *attr, int target)
{
	if (target != ZEND_ATTRIBUTE_TARGET_CLASS) {
		zend_error(E_COMPILE_ERROR, "Only classes can be marked with <<PhpAttribute>>");
	}
}

void zend_attribute_validate_phpcompilerattribute(zend_attribute *attr, int target)
{
	zend_error(E_COMPILE_ERROR, "The PhpCompilerAttribute can only be used by internal classes, use PhpAttribute instead");
}

void zend_attribute_validate_deprecated_attribute(zend_attribute *attr, int target)
{
	if (attr->argc > 1) {
		zend_error(E_COMPILE_ERROR, "<<Deprecated>> requires at most one argument, %d arguments given", attr->argc);
	}

	if (attr->argc == 1 && Z_TYPE(attr->argv[0]) != IS_STRING) {
		zend_error(E_COMPILE_ERROR, "<<Deprecated>>: Argument #1 ($message) must be of type string, %s given", zend_zval_type_name(&attr->argv[0]));
	}
}

ZEND_API zend_attributes_internal_validator zend_attribute_get_validator(zend_string *lcname)
{
	return zend_hash_find_ptr(&internal_validators, lcname);
}

ZEND_API void zend_attribute_free(zend_attribute *attr)
{
	uint32_t i;

	zend_string_release(attr->name);
	zend_string_release(attr->lcname);

	for (i = 0; i < attr->argc; i++) {
		zval_ptr_dtor(&attr->argv[i]);
	}

	efree(attr);
}

ZEND_API zend_attribute *zend_get_attribute(HashTable *attributes, zend_string *lcname, uint32_t offset)
{
	if (attributes) {
		zend_attribute *attr;

		ZEND_HASH_FOREACH_PTR(attributes, attr) {
			if (attr->offset == offset && zend_string_equals(attr->lcname, lcname)) {
				return attr;
			}
		} ZEND_HASH_FOREACH_END();
	}

	return NULL;
}

ZEND_API zend_attribute *zend_get_attribute_str(HashTable *attributes, const char *str, size_t len, uint32_t offset)
{
	if (attributes) {
		zend_attribute *attr;

		ZEND_HASH_FOREACH_PTR(attributes, attr) {
			if (attr->offset == offset && ZSTR_LEN(attr->lcname) == len) {
				if (0 == memcmp(ZSTR_VAL(attr->lcname), str, len)) {
					return attr;
				}
			}
		} ZEND_HASH_FOREACH_END();
	}

	return NULL;
}

ZEND_API void zend_compiler_attribute_register(zend_class_entry *ce, zend_attributes_internal_validator validator)
{
	zend_string *lcname = zend_string_tolower_ex(ce->name, 1);

	zend_hash_update_ptr(&internal_validators, lcname, validator);
	zend_string_release(lcname);
}

void zend_register_attribute_ce(void)
{
	zend_hash_init(&internal_validators, 8, NULL, NULL, 1);

	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "PhpAttribute", NULL);
	zend_ce_php_attribute = zend_register_internal_class(&ce);
	zend_ce_php_attribute->ce_flags |= ZEND_ACC_FINAL;

	zend_compiler_attribute_register(zend_ce_php_attribute, zend_attribute_validate_phpattribute);

	INIT_CLASS_ENTRY(ce, "PhpCompilerAttribute", NULL);
	zend_ce_php_compiler_attribute = zend_register_internal_class(&ce);
	zend_ce_php_compiler_attribute->ce_flags |= ZEND_ACC_FINAL;

	zend_compiler_attribute_register(zend_ce_php_compiler_attribute, zend_attribute_validate_phpcompilerattribute);

	INIT_CLASS_ENTRY(ce, "Deprecated", NULL);
	zend_ce_deprecated_attribute = zend_register_internal_class(&ce);
	zend_ce_php_compiler_attribute->ce_flags |= ZEND_ACC_FINAL;

	zend_compiler_attribute_register(zend_ce_deprecated_attribute, zend_attribute_validate_deprecated_attribute);
}
