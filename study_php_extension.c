/* study_php_extension extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_study_php_extension.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

PHPAPI void study_php_extension_var_dump(zval *struc, int level) /* {{{ */
{
	char tmp_str[PHP_DOUBLE_MAX_LENGTH];
	uint32_t count;
	HashTable *myht;
	zend_ulong num;
	zend_string *key;
	zval *val;
	zend_string *class_name;

	if (level > 1) {
		php_printf("%*c", level - 1, ' ');
	}

/* again: */
	switch(Z_TYPE_P(struc))
	{
		case IS_NULL:
			php_printf("NULL: null\n");
			break;
		case IS_TRUE:
			php_printf("BOOL: true\n");
			break;
		case IS_FALSE:
			php_printf("BOOL: false\n");
			break;
		case IS_LONG:
			php_printf("LONG: %ld\n", Z_LVAL_P(struc));
			break;
		case IS_DOUBLE:
			php_gcvt(Z_DVAL_P(struc), 17, '.', 'E', tmp_str);
			php_printf("DOUBLE: %s", tmp_str);
			if (zend_finite(Z_DVAL_P(struc)) && strchr(tmp_str, '.') == NULL) {
				PHPWRITE(".0", 2);
			}
			PHPWRITE("\n", 1);
			break;
		case IS_STRING:
			php_printf("STRING: value=\"");
			PHPWRITE(Z_STRVAL_P(struc), Z_STRLEN_P(struc));
			php_printf("\", length=%ld\n", Z_STRLEN_P(struc));
			break;
		case IS_RESOURCE: {
			const char *type_name = zend_rsrc_list_get_rsrc_type(Z_RES_P(struc));
			php_printf("RESOURCE: id=%d type=%s\n", Z_RES_P(struc)->handle, type_name ? type_name : "Unknown");
			break;
		}
		case IS_ARRAY:
			myht = Z_ARRVAL_P(struc);
			count = zend_array_count(myht);
			php_printf("ARRAY(%d) {\n", count);

			ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
				if (key == NULL) { /* numeric key */
					php_printf("%*c[" ZEND_LONG_FMT "]=>\n", level + 1, ' ', num);
				} else { /* string key */
					php_printf("%*c[\"", level + 1, ' ');
					PHPWRITE(ZSTR_VAL(key), ZSTR_LEN(key));
					php_printf("\"]=>\n");
				}
				study_php_extension_var_dump(val, level + 2);
			} ZEND_HASH_FOREACH_END();

			if (level > 1) {
				php_printf("%*c", level - 1, ' ');
			}
			PUTS("}\n");
			break;
		case IS_OBJECT:
			myht = zend_get_properties_for(struc, ZEND_PROP_PURPOSE_DEBUG);
			class_name = Z_OBJ_HANDLER_P(struc, get_class_name)(Z_OBJ_P(struc));

			php_printf("OBJECT(%s)#%d (%d) {\n", ZSTR_VAL(class_name), Z_OBJ_HANDLE_P(struc), myht ? zend_array_count(myht) : 0);

			if (myht) {
				ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
					zend_property_info *prop_info = NULL;

					if (Z_TYPE_P(val) == IS_INDIRECT) {
						val = Z_INDIRECT_P(val);
						if (key) {
							prop_info = zend_get_typed_property_info_for_slot(Z_OBJ_P(struc), val);
						}
					}

					if (!Z_ISUNDEF_P(val) || prop_info) { /* TODO: Is need Z_ISUNDEF_P? */
						const char *object_prop_name, *object_class_name;

						if (key == NULL) { /* numeric key */
							php_printf("%*c[" ZEND_LONG_FMT "]=>\n", level + 1, ' ', num);
						} else { /* string key */
							int unmangle = zend_unmangle_property_name(key, &object_class_name, &object_prop_name);
							php_printf("%*c[", level + 1, ' ');

							if (object_class_name && unmangle == SUCCESS) {
								if (object_class_name[0] == '*') {
									php_printf("\"%s\":protected", object_prop_name);
								} else {
									php_printf("\"%s\":\"%s\":private", object_prop_name, object_class_name);
								}
							} else {
								php_printf("\"");
								PHPWRITE(ZSTR_VAL(key), ZSTR_LEN(key));
								php_printf("\"");
							}
							ZEND_PUTS("]=>\n");
						}

						study_php_extension_var_dump(val, level + 2);
					}
				} ZEND_HASH_FOREACH_END();
				zend_release_properties(myht); /* must: Release myht */
			}

			zend_string_release_ex(class_name, 0);
			if (level > 1) {
				php_printf("%*c", level - 1, ' ');
			}
			PUTS("}\n");
			break;
		/* TODO: Should be add recursion protection.
		case IS_REFERENCE:
			struc = Z_REFVAL_P(struc);
			goto again;
			break;
		*/
		default:
			php_printf("UNKNOWN\n");
			break;
	}
}
/* }}} */

/* {{{ void study_php_extension_dump( [ mixed $var ] )
 */
PHP_FUNCTION(study_php_extension_dump)
{
	zval *zv_ptr;
	int argc, i;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_VARIADIC('+', zv_ptr, argc)
	ZEND_PARSE_PARAMETERS_END();

	for (i = 0; i < argc; i++) {
		study_php_extension_var_dump(&zv_ptr[i], 1);
	}
}

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(study_php_extension)
{
#if defined(ZTS) && defined(COMPILE_DL_STUDY_PHP_EXTENSION)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(study_php_extension)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "study_php_extension support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ study_php_extension_functions[]
 */
static const zend_function_entry study_php_extension_functions[] = {
	PHP_FE(study_php_extension_dump,		NULL)
	PHP_FE_END
};
/* }}} */

/* {{{ study_php_extension_module_entry
 */
zend_module_entry study_php_extension_module_entry = {
	STANDARD_MODULE_HEADER,
	"study_php_extension",					/* Extension name */
	study_php_extension_functions,			/* zend_function_entry */
	NULL,							/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(study_php_extension),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(study_php_extension),			/* PHP_MINFO - Module info */
	PHP_STUDY_PHP_EXTENSION_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_STUDY_PHP_EXTENSION
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(study_php_extension)
#endif
