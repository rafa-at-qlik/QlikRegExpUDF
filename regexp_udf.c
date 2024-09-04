#include <stdio.h>
#include <string.h>
#include <regex.h>
#include "ar_addon.h"
#include "ar_addon_transformation.h"


static void trans_regexp_udf(sqlite3_context *context, int argc, sqlite3_value **argv);

AR_AO_EXPORTED int ar_addon_init(AR_ADDON_CONTEXT *context)
{
	AR_AO_TRANSFORMATION_DEF *transdef = NULL;

	AR_AO_INIT(context);

	transdef = GET_AR_AO_TRANSFORMATION_DEF();
	transdef->displayName = "regexp_udf(X, Y)";
	transdef->functionName = "regexp_udf";
	transdef->description = "regexp function";
	transdef->func = trans_regexp_udf;
	transdef->nArgs = 2;
	AR_AO_REGISRATION->register_user_defined_transformation(transdef);
	return 0;

}

//Error handling
//Use sqlite3_result_error_code function to return an error.
//Three codes could have been used:
// SQLLITE_ERROR (1) - causes Replicate to skip the problematic data record
// SQLITE_CALLBACK_FATAL_ERROR (251) - causes Replicate to stop the task immediately with a fatal error
// SQLITE_CALLBACK_RECOVERABLE_ERROR (250) - causes Replicate to reattach the target endpoint or to stop the task with a recoverable error 
// Any other error code will be proceeded as SQLLITE_ERROR. 
// sqlite3_result_error function could have been used to return an error message. If this function is used without sqlite3_result_error_code, SQLLITE_ERROR is returned.   

static void trans_regexp_udf(sqlite3_context *context, int argc, sqlite3_value **argv)

{
	AR_AO_LOG->log_trace("enter trans_regexp_udf");
	if (argc >= 2) 
	{ // you should check that all the parameters declared in the function definition are provided
		regex_t regex;
		int reti;
		char *pszText = (char *)AR_AO_SQLITE->sqlite3_value_text(argv[0]);
		char *pszRegExp = (char *)AR_AO_SQLITE->sqlite3_value_text(argv[1]);
		char pRes[1000] = {0};

		// Compile the regular expression
		reti = regcomp(&regex, pszRegExp, 0);

		// Execute regular expression
		reti = regexec(&regex, pszText, 0, NULL, 0);

		// reti returns a 0 if it matches and 1 if it doesn't
		// which doesn't follow a boolean value
		// Adding New variable to return an 1 if it matches regexp and 0 if it doesn't
		int result_boolean;
		if (reti == 0) {
			result_boolean = 1;
		} else if (reti == 1) {
			result_boolean = 0;
		}

		// Cast or assign reti to pRes
		snprintf(pRes, sizeof(pRes), "%d", result_boolean);

		AR_AO_SQLITE->sqlite3_result_text(context, pRes, -1, SQLITE_TRANSIENT);
		AR_AO_LOG->log_trace("Before %s", "return");
		
	}
	else
	{ // should not occur but only if you do not declare the function correct
        AR_AO_LOG->log_error("The sqlite addon function trans_regexp_udf received an incorrect (%d instead of at least 2) number of parameters", argc);
		AR_AO_SQLITE->sqlite3_result_error(context, "incorrect parameter list", -1); 
		AR_AO_SQLITE->sqlite3_result_error_code(context, SQLITE_CALLBACK_FATAL_ERROR); 
	}

	AR_AO_LOG->log_trace("leave trans_regexp_udf");
}
