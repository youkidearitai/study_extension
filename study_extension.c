/*
 * study_extension extension for PHP
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_study_extension.h"
#include "zend_generators.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

PHPAPI void study_extension_var_dump(zval *struc, int level) /* {{{ */
{
	uint32_t count;
	HashTable *myht;
	zend_ulong num;
	zend_string *key;
	zval *val;
	zend_string *class_name;

	if (level > 1) {
		php_printf("%*c", level - 1, ' ');
	}

again:
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
			php_printf("LONG: " ZEND_LONG_FMT "\n", Z_LVAL_P(struc));
			break;
		case IS_DOUBLE:
			php_printf("DOUBLE: %.*G\n", (int) EG(precision), Z_DVAL_P(struc));
			break;
		case IS_STRING:
			php_printf("STRING: value=\"");
			PHPWRITE(Z_STRVAL_P(struc), Z_STRLEN_P(struc));
			php_printf("\", length=%zd\n", Z_STRLEN_P(struc));
			break;
		case IS_RESOURCE: {
			const char *type_name = zend_rsrc_list_get_rsrc_type(Z_RES_P(struc));
			php_printf("RESOURCE: id=%d type=%s\n", Z_RES_P(struc)->handle, type_name ? type_name : "Unknown");
			break;
		}
		case IS_ARRAY:
			myht = Z_ARRVAL_P(struc);
			/* recursion protection */
			if (level > 1 && !(GC_FLAGS(myht) & GC_IMMUTABLE)) {
				if (GC_IS_RECURSIVE(myht)) {
					PUTS("*RECURSION*\n");
					return;
				}
				GC_PROTECT_RECURSION(myht);
			}
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
				study_extension_var_dump(val, level + 2);
			} ZEND_HASH_FOREACH_END();

			if (level > 1 && !(GC_FLAGS(myht) & GC_IMMUTABLE)) {
				GC_UNPROTECT_RECURSION(myht);
			}

			if (level > 1) {
				php_printf("%*c", level - 1, ' ');
			}
			PUTS("}\n");
			break;
		case IS_OBJECT:
			/* recursion protection */
			if (Z_IS_RECURSIVE_P(struc)) {
				PUTS("*RECURSION*\n");
				return;
			}
			Z_PROTECT_RECURSION_P(struc);

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
							PUTS("]=>\n");
						}

						study_extension_var_dump(val, level + 2);
					}
				} ZEND_HASH_FOREACH_END();
				zend_release_properties(myht); /* must: Release myht */
			}

			zend_string_release_ex(class_name, 0);
			if (level > 1) {
				php_printf("%*c", level - 1, ' ');
			}
			PUTS("}\n");
			Z_UNPROTECT_RECURSION_P(struc); /* unprotect recursion */
			break;
		case IS_REFERENCE:
			struc = Z_REFVAL_P(struc);
			goto again;
			break;
		default:
			php_printf("UNKNOWN\n");
			break;
	}
}
/* }}} */

/* {{{ void study_extension_dump( mixed $var )
 */
PHP_FUNCTION(study_extension_dump)
{
	zval *zv_ptr;
	int argc, i;

	ZEND_PARSE_PARAMETERS_START(1, -1)
		Z_PARAM_VARIADIC('+', zv_ptr, argc)
	ZEND_PARSE_PARAMETERS_END();

	for (i = 0; i < argc; i++) {
		study_extension_var_dump(&zv_ptr[i], 1);
	}
}

static inline zend_bool study_skip_internal_handler(zend_execute_data *skip)
{
	return !(skip->func && ZEND_USER_CODE(skip->func->common.type))
		&& skip->prev_execute_data
		&& skip->prev_execute_data->func
		&& ZEND_USER_CODE(skip->prev_execute_data->func->common.type)
		&& skip->prev_execute_data->opline->opcode != ZEND_DO_FCALL
		&& skip->prev_execute_data->opline->opcode != ZEND_DO_ICALL
		&& skip->prev_execute_data->opline->opcode != ZEND_DO_UCALL
		&& skip->prev_execute_data->opline->opcode != ZEND_DO_FCALL_BY_NAME
		&& skip->prev_execute_data->opline->opcode != ZEND_INCLUDE_OR_EVAL;
}

/* {{{ void study_extension_print_backtrace( [ int options ] )
 */
PHP_FUNCTION(study_extension_print_backtrace)
{
	zend_execute_data *ptr, *skip, *call;
	zend_function *func;
	zend_long options = 0;
	zend_long limit = 0;
	zval arg_array;

	zend_object *object;

	int lineno;
	int frameno = 0;

	char *call_type;
	const char *filename;
	const char *function_name;
	zend_string *class_name = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|ll", &options, &limit) == FAILURE) {
		return;
	}

	ZVAL_UNDEF(&arg_array);
	ptr = EX(prev_execute_data);

	call = ptr;
	ptr = ptr->prev_execute_data;

	while (ptr && (limit == 0 || frameno < limit)) {
		frameno++;
		class_name = NULL;
		call_type = NULL;

		ZVAL_UNDEF(&arg_array);
		ptr = zend_generator_check_placeholder_frame(ptr);
		skip = ptr;

		if (study_skip_internal_handler(skip)) {
			skip = skip->prev_execute_data;
		}

		if (skip->func && ZEND_USER_CODE(skip->func->common.type)) {
			filename = ZSTR_VAL(skip->func->op_array.filename);
			if (skip->opline->opcode == ZEND_HANDLE_EXCEPTION) {
				if (EG(opline_before_exception)) {
					lineno = EG(opline_before_exception)->lineno;
				} else {
					lineno = skip->func->op_array.line_end;
				}
			} else {
				lineno = skip->opline->lineno;
			}
		} else {
			filename = NULL;
			lineno = 0;
		}

		object = (Z_TYPE(call->This) == IS_OBJECT) ? Z_OBJ(call->This) : NULL;

		if (call->func) {
			zend_string *zend_function_name;
			func = call->func;

			if (func->common.scope && func->common.scope->trait_aliases) {
				zend_function_name = zend_resolve_method_name(object ? object->ce : func->common.scope, func);
			} else {
				zend_function_name = func->common.function_name;
			}

			if (zend_function_name != NULL) {
				function_name = ZSTR_VAL(zend_function_name);
			} else {
				function_name = NULL;
			}
		} else {
			func = NULL;
			function_name = NULL;
		}

		if (function_name) {
			if (object) {
				if (func->common.scope) {
					class_name = func->common.scope->name;
				} else if (object->handlers->get_class_name == zend_std_get_class_name) {
					class_name = object->ce->name;
				} else {
					class_name = object->handlers->get_class_name(object);
				}

				call_type = "->";
			} else if (func->common.scope) {
				class_name = func->common.scope->name;
				call_type = "::";
			} else {
				class_name = NULL;
				call_type = NULL;
			}
		} else {
			if (!ptr->func || !ZEND_USER_CODE(ptr->func->common.type) || ptr->opline->opcode != ZEND_INCLUDE_OR_EVAL) {
				function_name = "unknown";
			} else {
				switch (ptr->opline->extended_value) {
					case ZEND_EVAL:
						function_name = "eval";
						break;
					case ZEND_INCLUDE:
						function_name = "include";
						break;
					case ZEND_REQUIRE:
						function_name = "require";
						break;
					default:
						function_name = "unknown";
						break;
				}
			}
			call_type = NULL;
		}

		php_printf("function: ");
		if (class_name) {
			PUTS(ZSTR_VAL(class_name));
			PUTS(call_type);
		}
		php_printf("%s\t", function_name);
		if (filename) {
			php_printf("filename: %s:%d\n", filename, lineno);
		} else {
			// ex: array_walk, preg_replace etc
			PUTS("\n");
		}

		call = skip;
		ptr = skip->prev_execute_data;
	}

}

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(study_extension)
{
#if defined(ZTS) && defined(COMPILE_DL_STUDY_EXTENSION)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(study_extension)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "study_extension support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO(arginfo_study_extension_dump, 0)
	ZEND_ARG_VARIADIC_INFO(0, vars)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_extension_print_backtrace, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
	ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()

/* }}} */


/* {{{ study_extension_functions[]
 */
static const zend_function_entry study_extension_functions[] = {
	PHP_FE(study_extension_dump,		arginfo_study_extension_dump)
	PHP_FE(study_extension_print_backtrace,		arginfo_study_extension_print_backtrace)
	PHP_FE_END
};
/* }}} */

/* {{{ study_extension_module_entry
 */
zend_module_entry study_extension_module_entry = {
	STANDARD_MODULE_HEADER,
	"study_extension",					/* Extension name */
	study_extension_functions,			/* zend_function_entry */
	NULL,							/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(study_extension),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(study_extension),			/* PHP_MINFO - Module info */
	PHP_STUDY_EXTENSION_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_STUDY_EXTENSION
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(study_extension)
#endif

/*
 * vim: noexpandtab tabstop=4 shiftwidth=4
 */
