/* study_extension extension for PHP */

#ifndef PHP_STUDY_EXTENSION_H
# define PHP_STUDY_EXTENSION_H

extern zend_module_entry study_extension_module_entry;
# define phpext_study_extension_ptr &study_extension_module_entry

# define PHP_STUDY_EXTENSION_VERSION "0.2.0"

static const zend_function_entry class_study_standard_class_methods[] = {
    ZEND_FE_END
};

PHPAPI void study_extension_var_dump(zval *struc, int level, int escape);
static ZEND_COLD int study_php_info_print_html_esc(const char *str, size_t len);

#define TEKITOH_MEMDHOI_INFO_BACKGROUND_IMAGE "data:image/png;base64,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"

#define TEKITOH_MEMDHOI_INFO_BORDER_IMAGE "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAACWUlEQVR4nO3XW27DMAwF0Swh+99s+1UgaF62RJG0ZgbQrxPrnqLt7cfQ3aq/gNUWDuB+v29/dioEQPUg1RCujG4aQPUItBPdFIDqyyCfqIYBVF+AJwbBaQDVL+2JxXAKQPVLeuIRHAZQ/XKeNQgOAcgU2bHqUUsBkIf/X+b7twDg+D1aucFbAI7frxVbvATg+H1rB8Byi97mCYDj968FAKstaqchAFafAKwGgPVqFoEALp4A4KUDEEG/Zjbz38ANEgC8UACzD7T8BGCxAGYeaDUJAJ4A4I3+2hbAJoUC8I/A6yUAeAIwAdALAeD4100A8AQATwDwUgBY3wQATwDwBABPAPAEAG70vzcBbJIA4AkA3uhuAtgkAcATADwBwBMAPAHAEwA8AcATADwBwBMAPAHAEwA8AYD7tpkANk8A8AQAb2YzAWyQAOAJAJ4A4AkAngDgCQCeAOAJAJ4A4AkAngDgCQCeAOAJAJ4A4AkAXhiAbw8SQM8EAE8A8AQALw2A9UwA8AQATwDwBABPAPAEAE8A8AQATwDwBABPAPAEAC8NgAh6JgB4AoAnAHhhAGYfZjWlAhBBvwQATwDwQgHMPtBym/2BFcDFKwEggh4d2UkAm3Z0/GUARFBb1EZTAIRQU+Q2LwGc/RAh5BW9RygAIaxr1QZvAYx8qBDiyrr7jwAivsiOGKLuZNU501cAWS/cteoxV9/jIQBXvAjqWQZACP3PSKcBiKDnGW0IgBD6nNmmADxWfRG0E1UYgL+qL2bXs6pwAI9VX9qVh8lqKYBPVQ9HHPtVZQCsR7/Q1BYCRFMA9AAAAABJRU5ErkJggg=="

# if defined(ZTS) && defined(COMPILE_DL_STUDY_EXTENSION)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_STUDY_EXTENSION_H */
