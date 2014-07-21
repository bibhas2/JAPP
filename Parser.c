#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "Parser.h"

#define FAIL(cond, msg) if (cond) {printf("%s %s:%d\n", msg, __FILE__, __LINE__); return NULL;}

static int printDict(const char *key, void *value) {
	printf("Key: \"%s\"\n", key);
	jsonPrintObject((JSONObject*) value);

	return 1;
}

void
jsonPrintObject(JSONObject *o) {
	switch (o->type) {
		case JSON_STRING:
			puts("Printing String...");
			printf("\"%s\"\n", 
				stringAsCString(o->value.string));
			break;
		case JSON_OBJECT:
			puts("Printing object...");
			dictionaryIterate(o->value.object,
				printDict);
			break;
		default:
			printf("Unsupported type.");
	}
}

JSONParser *newJSONParser(String *dataToParse) {
	JSONParser *parser = malloc(sizeof(JSONParser));

	parser->data = dataToParse;
	parser->position = 0;

	return parser;
}
void deleteJSONParser(JSONParser *parser) {
	parser->data = NULL;
	parser->position = 0;

	free(parser);
}

static JSONObject *
getArrayObject(JSONObject *o, int index) {
	assert(o->type == JSON_ARRAY);

	return arrayGet(o->value.array, index);
}

String *jsonGetStringAt(JSONObject *a, int index) {
	JSONObject *child = getArrayObject(a, index);
	
	if (child == NULL) {
		return NULL; //Not found
	}
	assert(child->type == JSON_STRING);

	return child->value.string;
}

double jsonGetNumberAt(JSONObject *a, int index) {
	JSONObject *child = getArrayObject(a, index);
	
	if (child == NULL) {
		return 0.0; //Not found
	}
	assert(child->type == JSON_NUMBER);

	return child->value.number;
}

JSONObject *jsonGetObjectAt(JSONObject *a, int index) {
	JSONObject *child = getArrayObject(a, index);
	
	if (child == NULL) {
		return NULL; //Not found
	}

	assert(child->type == JSON_OBJECT);

	return child;
}

JSONObject *jsonGetArrayAt(JSONObject *a, int index) {
	JSONObject *child = getArrayObject(a, index);
	
	if (child == NULL) {
		return NULL; //Not found
	}

	assert(child->type == JSON_ARRAY);

	return child;
}

bool jsonGetBooleanAt(JSONObject *a, int index) {
	JSONObject *child = getArrayObject(a, index);
	
	if (child == NULL) {
		return false; //Not found
	}

	assert(child->type == JSON_BOOLEAN);

	return child->value.booleanValue;
}

bool jsonIsNullAt(JSONObject *a, int index) {
	JSONObject *child = getArrayObject(a, index);
	
	if (child == NULL) {
		return true; //Not found
	}

	return(child->type == JSON_NULL);
}

String *jsonGetString(JSONObject *o, const char *name) {
	assert(o->type == JSON_OBJECT);

	JSONObject *child = dictionaryGet(o->value.object, name);
	
	if (child == NULL) {
		return NULL; //Not found
	}
	assert(child->type == JSON_STRING);

	return child->value.string;
}

double jsonGetNumber(JSONObject *o, const char *name) {
	assert(o->type == JSON_OBJECT);

	JSONObject *child = dictionaryGet(o->value.object, name);
	
	if (child == NULL) {
		return 0.0; //Not found
	}

	assert(child->type == JSON_NUMBER);

	return child->value.number;
}

JSONObject *jsonGetObject(JSONObject *o, const char *name) {
	assert(o->type == JSON_OBJECT);

	JSONObject *child = dictionaryGet(o->value.object, name);
	
	if (child == NULL) {
		return NULL; //Not found
	}

	assert(child->type == JSON_OBJECT);

	return child;
}

Array *jsonGetArray(JSONObject *o, const char *name) {
	assert(o->type == JSON_OBJECT);

	JSONObject *child = dictionaryGet(o->value.object, name);
	
	if (child == NULL) {
		return NULL; //Not found
	}

	assert(child->type == JSON_ARRAY);

	return child->value.array;
}

bool jsonGetBoolean(JSONObject *o, const char *name) {
	assert(o->type == JSON_OBJECT);

	JSONObject *child = dictionaryGet(o->value.object, name);
	
	if (child == NULL) {
		return false; //Not found
	}

	assert(child->type == JSON_BOOLEAN);

	return child->value.booleanValue;
}

bool jsonIsNull(JSONObject *o, const char *name) {
	assert(o->type == JSON_OBJECT);

	JSONObject *child = dictionaryGet(o->value.object, name);
	
	if (child == NULL) {
		return true; //Not found
	}

	return(child->type == JSON_NULL);
}

static char peek(JSONParser *parser) {
	if (parser->position >= parser->data->length) {
		return 0;
	}
	return stringGetChar(parser->data, parser->position);
}
static char pop(JSONParser *parser) {
	if (parser->position >= parser->data->length) {
		return 0;
	}

	parser->position += 1;

	return stringGetChar(parser->data, parser->position - 1);
}
static void putback(JSONParser *parser) {
	assert(parser->position > 0);
	parser->position -= 1;
}

JSONObject *newJSONObject(JSONType type) {
	JSONObject *o = malloc(sizeof(JSONObject));

	o->type = type;

	return o;
}

static void eatSpace(JSONParser *parser) {
	char ch = pop(parser);

	if (ch == 0) return;

	while (isspace(ch)) {
		ch = pop(parser);

		if (ch == 0) return;
	}
	putback(parser);
}

static JSONObject* parseValue(JSONParser *parser);

static String* parseString(JSONParser *parser) {
	eatSpace(parser);

	char ch = pop(parser);

	FAIL(ch == 0, "Premature end of JSON string.");
	assert(ch == '"');

	String *s = newString();

	while ((ch = pop(parser)) != '"') {
		FAIL(ch == 0, "Premature end of JSON string.");

		stringAppendChar(s, ch);
	}

	return s;
}

static double parseNumber(JSONParser *parser) {
	return 0.0;
}

static JSONObject *parseObject(JSONParser *parser) {
	JSONObject *o = newJSONObject(JSON_OBJECT);
	Dictionary *d = newDictionary();
	String *name = NULL;

	o->value.object = d;

	char ch = pop(parser);

	FAIL(ch == 0, "Premature end of JSON string.");
	FAIL(ch != '{', "Invalid JSON object format.");

	while ((ch = pop(parser)) != '}') {
		FAIL(ch == 0, "Premature end of JSON string.");

		if (ch == '"') {
			putback(parser);
			name = parseString(parser);
		} else if (ch == ':') {
			JSONObject *val = parseValue(parser);
			dictionaryPut(d, 
				stringAsCString(name), val);
			deleteString(name);
			name = NULL;
		}
	}

	return o;
}

static Array* parseArray(JSONParser *parser) {
	eatSpace(parser);

	char ch = pop(parser);

	FAIL(ch == 0, "Premature end of JSON string.");
	FAIL(ch != '[', "Invalid JSON array.");

	Array *a = newArray(25);

	while ((ch = pop(parser)) != ']') {
		FAIL(ch == 0, "Premature end of JSON string.");
		
		putback(parser);
		JSONObject *o = parseValue(parser);
		if (o != NULL) {
			arrayAdd(a, o);
		} 
		eatSpace(parser);
		if (pop(parser) != ',') {
			putback(parser);
		}
	}

	return a;
}

static JSONObject* parseValue(JSONParser *parser) {
	eatSpace(parser);

	char ch = peek(parser);

	FAIL(ch == 0, "Premature end of JSON string.");

	if (ch == '"') {
		String *str = parseString(parser);
		JSONObject *o = newJSONObject(JSON_STRING);
		o->value.string = str;
		return o;
	} else if (ch == '{') {
		return parseObject(parser);
	} else if (ch == '[') {
		Array *a = parseArray(parser);
		JSONObject *o = newJSONObject(JSON_ARRAY);
		o->value.array = a;

		return o;
	} else if (isdigit(ch)) {
		JSONObject *o = newJSONObject(JSON_NUMBER);
		o->value.number = parseNumber(parser);

		return o;
	} //add true, false, null

	return NULL;
}

JSONObject *jsonParseObject(JSONParser *parser) {
	eatSpace(parser);

	char ch = peek(parser);

	FAIL(ch == 0, "Premature end of JSON string.");
	FAIL(ch != '{', "Not a valid JSON object.");

	return parseObject(parser);
}

JSONObject *jsonParseArray(JSONParser *parser) {
	eatSpace(parser);

	char ch = peek(parser);

	FAIL(ch == 0, "Premature end of JSON string.");
	FAIL(ch != '[', "Not a valid JSON array.");

	Array *a = parseArray(parser);
	JSONObject *o = newJSONObject(JSON_ARRAY);
	o->value.array = a;

	return o;
}
