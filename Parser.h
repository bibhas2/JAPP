#include <stdbool.h>
#include "../Cute/String.h"
#include "../Cute/Dictionary.h"
#include "../Cute/Array.h"

typedef enum _ErrorCode {
	ERROR_NONE,
	ERROR_INVALID_TYPE,
	ERROR_SYNTAX
} ErrorCode;

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
	int line;
	const char *errorMessage;
	int errorLine;
	ErrorCode errorCode;
	JSONObject *root;
} JSONParser;

JSONParser *newJSONParser();
void deleteJSONParser(JSONParser *parser);
JSONObject *jsonParse(JSONParser *parser, String *stringToParse);
JSONObject *jsonParseCString(JSONParser *parser, const char *stringToParse);

//Get named properties of a JSON Object
String *jsonGetString(JSONObject *o, const char *name);
const char *jsonGetCString(JSONObject *o, const char *name);
double jsonGetNumber(JSONObject *o, const char *name);
JSONObject *jsonGetObject(JSONObject *o, const char *name);
JSONObject *jsonGetArray(JSONObject *o, const char *name);
bool jsonGetBoolean(JSONObject *o, const char *name);
bool jsonIsNull(JSONObject *o, const char *name);
JSONObject *jsonGetObjectByPath(JSONObject *o, const char *path);
JSONObject *jsonGetArrayByPath(JSONObject *o, const char *path);

//Get the number of items in a JSON array
int jsonGetArrayLength(JSONObject *a);
//Get indexed properties of a JSON array
String *jsonGetStringAt(JSONObject *a, int index);
const char *jsonGetCStringAt(JSONObject *a, int index);
double jsonGetNumberAt(JSONObject *a, int index);
JSONObject *jsonGetObjectAt(JSONObject *a, int index);
JSONObject *jsonGetArrayAt(JSONObject *a, int index);
bool jsonGetBooleanAt(JSONObject *a, int index);
bool jsonIsNullAt(JSONObject *a, int index);

void jsonPrintObject(JSONObject *o);
