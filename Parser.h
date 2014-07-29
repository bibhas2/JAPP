#include <stdbool.h>
#include "../Cute/String.h"
#include "../Cute/Dictionary.h"
#include "../Cute/Array.h"

typedef enum _JSONType {
	JSON_STRING,
	JSON_NUMBER,
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_BOOLEAN,
	JSON_NULL
} JSONType;

typedef struct _JSONObject {
	JSONType type;
	union {
		String *string;
		double number;
		Dictionary *object;
		Array *array;
		bool booleanValue;
		bool isNull;
	} value;
} JSONObject;

typedef struct _JSONParser {
	String *data;
	int position;
	JSONObject *root;
} JSONParser;

JSONParser *newJSONParser();
void deleteJSONParser(JSONParser *parser);
JSONObject *jsonParse(JSONParser *parser, String *stringToParse);
JSONObject *jsonParseCString(JSONParser *parser, const char *stringToParse);

String *jsonGetString(JSONObject *o, const char *name);
const char *jsonGetCString(JSONObject *o, const char *name);
double jsonGetNumber(JSONObject *o, const char *name);
JSONObject *jsonGetObject(JSONObject *o, const char *name);
JSONObject *jsonGetArray(JSONObject *o, const char *name);
bool jsonGetBoolean(JSONObject *o, const char *name);
bool jsonIsNull(JSONObject *o, const char *name);

String *jsonGetStringAt(JSONObject *a, int index);
const char *jsonGetCStringAt(JSONObject *a, int index);
double jsonGetNumberAt(JSONObject *a, int index);
JSONObject *jsonGetObjectAt(JSONObject *a, int index);
JSONObject *jsonGetArrayAt(JSONObject *a, int index);
bool jsonGetBooleanAt(JSONObject *a, int index);
bool jsonIsNullAt(JSONObject *a, int index);

void jsonPrintObject(JSONObject *o);
