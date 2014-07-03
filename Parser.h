#include <stdbool.h>
#include <String.h>
#include <Dictionary.h>
#include <Array.h>

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
	} value;
} JSONObject;

typedef struct _JSONParser {
	String *data;
	int position;
} JSONParser;

JSONParser *newJSONParser(String *dataToParse);
void deleteJSONParser(JSONParser *parser);
JSONObject *jsonParseObject(JSONParser *parser);
Array *jsonParseArray(JSONParser *parser);

String *jsonGetString(JSONObject *o, const char *name);
double jsonGetNumber(JSONObject *o, const char *name);
Dictionary *jsonGetObject(JSONObject *o, const char *name);
Array *jsonGetArray(JSONObject *o, const char *name);
bool jsonGetBoolean(JSONObject *o, const char *name);
bool jsonIsNull(JSONObject *o, const char *name);
void jsonPrintObject(JSONObject *o);
