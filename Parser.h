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
JSONObject *jsonParse(JSONParser *parser);
String *jsonGetString(JSONObject *o);
double jsonGetNumber(JSONObject *o);
Dictionary *jsonGetObject(JSONObject *o);
Array *jsonGetArray(JSONObject *o);
bool jsonGetBoolean(JSONObject *o);
bool jsonIsNull(JSONObject *o);
void jsonPrintObject(JSONObject *o);
