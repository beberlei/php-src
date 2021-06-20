/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 5cb75e0f0869651dd616b11c3bd0d407deb3c6df */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pdo_drivers, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()


ZEND_FUNCTION(pdo_drivers);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(pdo_drivers, arginfo_pdo_drivers)
	ZEND_FE_END
};


static const zend_function_entry class_PDOException_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_PDOException(zend_class_entry *class_entry_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "PDOException", class_PDOException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_RuntimeException);

	zval property_errorInfo_default_value;
	ZVAL_NULL(&property_errorInfo_default_value);
	zend_string *property_errorInfo_name = zend_string_init("errorInfo", sizeof("errorInfo") - 1, 1);
	zend_declare_typed_property(class_entry, property_errorInfo_name, &property_errorInfo_default_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY|MAY_BE_NULL));
	zend_string_release(property_errorInfo_name);

	return class_entry;
}
