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
			php_printf("ARRAY: hashtable=%p\n", Z_ARRVAL_P(struc));
			break;
		case IS_OBJECT:
			php_printf("OBJECT: ???\n");
			break;
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
