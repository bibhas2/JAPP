#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
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

JSONParser *newJSONParser() {
	JSONParser *parser = malloc(sizeof(JSONParser));

	parser->data = NULL;
	parser->root = NULL;
	parser->position = 0;

	return parser;
}

static void deleteJSONObject(JSONObject *o);

void clearParser(JSONParser *parser) {
	if (parser->root != NULL) {
		deleteJSONObject(parser->root);
		parser->root = NULL;
	}

	parser->data = NULL;
	parser->position = 0;
}

static int delete_object_properties(const char *key, void *value) {
	deleteJSONObject((JSONObject*) value);

	return 1;
}

static void deleteJSONObject(JSONObject *o) {
	if (o->type == JSON_STRING) {
		deleteString(o->value.string);
		o->value.string = NULL;
	} else if (o->type == JSON_ARRAY) {
		for (int i = 0; i < o->value.array->length; ++i) {
			JSONObject *item = arrayGet(o->value.array, i);
			deleteJSONObject(item);
		}
		deleteArray(o->value.array);
		o->value.array = NULL;
	} else if (o->type == JSON_OBJECT) {
		dictionaryIterate(o->value.object, delete_object_properties);
		deleteDictionary(o->value.object);
		o->value.object = NULL;
	}

	free(o);
}

void deleteJSONParser(JSONParser *parser) {
	clearParser(parser);

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

	if (child->type != JSON_NULL) {
		return false;
	}

	return child->value.isNull;
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

JSONObject *jsonGetArray(JSONObject *o, const char *name) {
	assert(o->type == JSON_OBJECT);

	JSONObject *child = dictionaryGet(o->value.object, name);
	
	if (child == NULL) {
		return NULL; //Not found
	}

	assert(child->type == JSON_ARRAY);

	return child;
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

	if (child->type != JSON_NULL) {
		return false;
	}

	return child->value.isNull;
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
	JSONObject *o = calloc(1, sizeof(JSONObject));

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
		if (ch == 0) {
			puts("Premature end of JSON string.");
			deleteString(s);
			return NULL;
		}

		if (ch == '\\') {
			//Escape handling

			char escaped = pop(parser);

			if (escaped == 0) {
				puts("Premature end of JSON string.");
				deleteString(s);

				return NULL;
			}

			if (escaped == 't') {
				ch = '\t';
			} else if (escaped == 'r') {
				ch = '\r';
			} else if (escaped == 'n') {
				ch = '\n';
			} else if (escaped == 'b') {
				ch = '\b';
			} else if (escaped == '"') {
				ch = '"';
			} else if (escaped == '\\') {
				ch = '\\';
			}
		}

		stringAppendChar(s, ch);
	}

	return s;
}

/**
 * Reads the text for next number, boolean and null value.
 */
static String *readValueToken(JSONParser *parser) {
	eatSpace(parser);

	String *s = newString();

	while (1) {
		char ch = pop(parser);

		if (ch == 0) {
			puts("Premature end of JSON string.");
			deleteString(s);
			return NULL;
		} else if (ch == '}' || ch == ']' || ch == ',') {
			putback(parser);

			break;
		}

		stringAppendChar(s, ch);
	}

	stringTrim(s);

	return s;
}

static double parseNumber(JSONParser *parser) {
	String *s = readValueToken(parser);

	if (s == NULL) {
		return 0.0;
	}

	double d = 0.0;
	const char *str = stringAsCString(s);

	if (sscanf(str, "%le", &d) == 0) {
		printf("Failed to parse number from: %s\n", str);

		d = 0.0;
	}

	deleteString(s);

	return d;
}

static bool parseBool(JSONParser *parser) {
	String *s = readValueToken(parser);

	if (s == NULL) {
		return false;
	}

	bool val = false;
	
	if (strcmp(stringAsCString(s), "true") == 0) {
		val = true;
	} else if (strcmp(stringAsCString(s), "false") == 0) {
		val = false;
	} else {
		//Error
		printf("Invalid boolean value: %s\n", stringAsCString(s));
	}

	deleteString(s);

	return val;
}

static bool parseNull(JSONParser *parser) {
	String *s = readValueToken(parser);

	if (s == NULL) {
		return false;
	}

	bool val = strcmp(stringAsCString(s), "null") == 0;

	deleteString(s);

	return val;
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
	} else if (ch == 't') {
		JSONObject *o = newJSONObject(JSON_BOOLEAN);
		o->value.booleanValue = parseBool(parser);

		return o;
	} else if (ch == 'f') {
		JSONObject *o = newJSONObject(JSON_BOOLEAN);
		o->value.booleanValue = parseBool(parser);

		return o;
	} else if (ch == 'n') {
		JSONObject *o = newJSONObject(JSON_NULL);
		o->value.isNull = parseNull(parser);

		return o;
	}

 //add true, false, null

	return NULL;
}

JSONObject *jsonParse(JSONParser *parser, String *stringToParse) {
	clearParser(parser);

	parser->data = stringToParse;

	eatSpace(parser);

	char ch = peek(parser);

	if (ch == '{') {
		parser->root = parseObject(parser);
	} else if (ch == '[') {
		Array *a = parseArray(parser);
		JSONObject *o = newJSONObject(JSON_ARRAY);
		o->value.array = a;

		parser->root = o;
	}

	return parser->root;
}
