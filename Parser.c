#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "Parser.h"

#define FAIL(cond, p, code, msg) if (cond) {save_error(p, code, msg); return NULL;}

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

	assert(parser != NULL);

	parser->data = NULL;
	parser->root = NULL;
	parser->position = 0;
	parser->errorLine = 0;
	parser->errorCode = ERROR_NONE;
	parser->errorMessage = NULL;
	parser->onPropertyParsed = NULL;
	parser->onValueParsed = NULL;

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
	parser->errorLine = 0;
	parser->errorCode = ERROR_NONE;
	parser->errorMessage = NULL;
}

static int delete_object_properties(const char *key, void *value) {
	deleteJSONObject((JSONObject*) value);

	return 1;
}

void jsonClear(JSONObject *o) {
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

	o->type = JSON_UNDEFINED;
}

static void deleteJSONObject(JSONObject *o) {
	jsonClear(o);

	free(o);
}

void deleteJSONParser(JSONParser *parser) {
	clearParser(parser);

	free(parser);
}

static void onPropertyParsed(JSONParser *parser, String *name, JSONObject *val) {
	if (parser->onPropertyParsed != NULL) {
		parser->onPropertyParsed(parser, name, val);
	}
}
static void onValueParsed(JSONParser *parser, JSONObject *val) {
	if (parser->onValueParsed != NULL) {
		parser->onValueParsed(parser, val);
	}
}

void save_error(JSONParser *p, ErrorCode code, const char *msg) {
	p->errorCode = code;
	p->errorMessage = msg;
}

static JSONObject *
getArrayObject(JSONObject *o, int index) {
	assert(o->type == JSON_ARRAY);

	return arrayGet(o->value.array, index);
}

int jsonGetArrayLength(JSONObject *a) {
	assert(a->type == JSON_ARRAY);

	return a->value.array->length;
}

String *jsonGetStringAt(JSONObject *a, int index) {
	JSONObject *child = getArrayObject(a, index);
	
	if (child == NULL) {
		return NULL; //Not found
	}
	assert(child->type == JSON_STRING);

	return child->value.string;
}

const char *jsonGetCStringAt(JSONObject *a, int index) {
	return stringAsCString(jsonGetStringAt(a, index));
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

const char *jsonGetCString(JSONObject *o, const char *name) {
	return stringAsCString(jsonGetString(o, name));
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

static JSONObject *_jsonGetByPath(JSONObject *o, const char *path) {
	String *segment = newString();

	while (*path != '\0') {
		if (*path == '/') {
			assert(o->type == JSON_OBJECT); //Parent must be object
			o = dictionaryGet(o->value.object, 
				stringAsCString(segment));	
			segment->length = 0; //Reset
		} else {
			stringAppendChar(segment, *path);
		}
		++path;
	}
	if (segment->length > 0) {
		assert(o->type == JSON_OBJECT); //Parent must be object
		o = dictionaryGet(o->value.object, 
			stringAsCString(segment));	
	}

	deleteString(segment);

	return o;
}

/**
 * Find an object  that descends from the given object by the object name
 * hierarchy. All properties in the hierarchy must be objects (ie, can not
 * be array).
 */
JSONObject *jsonGetObjectByPath(JSONObject *o, const char *path) {
	o = _jsonGetByPath(o, path);

	if (o == NULL) {
		return NULL;
	}

	assert(o->type == JSON_OBJECT);

	return o;
}

/**
 * Find an array  that descends from the given object by the object name
 * hierarchy. All properties in the hierarchy, except fo the last one,
 * must be objects (ie, can not * be array). The last property in the
 * path must be an array.
 */
JSONObject *jsonGetArrayByPath(JSONObject *o, const char *path) {
	o = _jsonGetByPath(o, path);

	if (o == NULL) {
		return NULL;
	}

	assert(o->type == JSON_ARRAY);

	return o;
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

	char ch = stringGetChar(parser->data, parser->position - 1);

	if (ch == '\n') {
		parser->errorLine += 1;
	}

	return ch;
}

static void putback(JSONParser *parser) {
	assert(parser->position > 0);
	parser->position -= 1;

	char ch = stringGetChar(parser->data, parser->position);

	/*
	 * If we are putting back new line decrease line number.
	 */
	if (ch == '\n') {
		parser->errorLine -= 1;
	}
}

JSONObject *newJSONObject(JSONType type) {
	JSONObject *o = calloc(1, sizeof(JSONObject));

	assert(o != NULL);

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

/*
 * Code stolen from: http://stackoverflow.com/a/4609989/1036017
 */
static void unicodeToUTF8(int unicode, char *out, int *bytesWritten) {
	char *pos = out;

	if (unicode<0x80) *pos++=unicode;
	else if (unicode<0x800) *pos++=192+unicode/64, *pos++=128+unicode%64;
	else if (unicode-0xd800u<0x800) {}
	else if (unicode<0x10000) *pos++=224+unicode/4096, *pos++=128+unicode/64%64, *pos++=128+unicode%64;
	else if (unicode<0x110000) *pos++=240+unicode/262144, *pos++=128+unicode/4096%64, *pos++=128+unicode/64%64, *pos++=128+unicode%64;


	*bytesWritten = (pos - out);
}


static String* parseString(JSONParser *parser) {
	eatSpace(parser);

	char ch = pop(parser);

	FAIL(ch == 0, parser, ERROR_SYNTAX, "Premature end of document while parsing string.");
	assert(ch == '"');

	String *s = newString();

	while ((ch = pop(parser)) != '"') {
		if (ch == 0) {
			save_error(parser, ERROR_SYNTAX, "Premature end of document while parsing string.");
			deleteString(s);
			return NULL;
		}

		if (ch == '\\') {
			//Escape handling

			char escaped = pop(parser);

			if (escaped == 0) {
				save_error(parser, ERROR_SYNTAX, "Invalid escaped character in string.");
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
			} else if (escaped == 'u') {
				//Unicode escape. Must be 2 hex digits.
				char in[5];
				
				in[0] = pop(parser);
				in[1] = pop(parser);
				in[2] = pop(parser);
				in[3] = pop(parser);
				in[4] = '\0';

				int unicode;
				sscanf(in, "%04X", &unicode);

				char out[4];
				int bytesWritten;

				unicodeToUTF8(unicode, out, &bytesWritten);

				if (bytesWritten == 0) {
					save_error(parser, ERROR_SYNTAX, "Failed to convert UNICODE to UTF-8.");
					continue;
				}

				for (int i = 0; i < bytesWritten; ++i) {
					stringAppendChar(s, out[i]);
				}

				continue;
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
			save_error(parser, ERROR_SYNTAX, "Premature end of document while looking for a value.");
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
		save_error(parser, ERROR_SYNTAX, "Failed to parse number.");

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
		save_error(parser, ERROR_SYNTAX, "Invalid boolean value.");
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
	char ch = pop(parser);

	FAIL(ch == 0, parser, ERROR_SYNTAX, "Premature end of document while parsing an object.");
	FAIL(ch != '{', parser, ERROR_SYNTAX, "Object does not start with '{'");

	JSONObject *o = newJSONObject(JSON_OBJECT);
	Dictionary *d = newDictionary();
	String *name = NULL;

	o->value.object = d;

	while (1) {
		eatSpace(parser);
		ch = pop(parser);

		if (ch == 0) {
			save_error(parser, ERROR_SYNTAX, "Premature end of document while parsing an object.");
			break;
		}
		if (ch == '}') {
			//End of object
			break;
		} else if (ch == '"') {
			putback(parser);
			name = parseString(parser);
		} else if (ch == ':') {
			JSONObject *val = parseValue(parser);
			dictionaryPut(d, 
				stringAsCString(name), val);
			onPropertyParsed(parser, name, val);
			deleteString(name);
			name = NULL;
		} else if (ch == ',') {
			//End of a property. Nothing to do here.
		} else {
			save_error(parser, ERROR_SYNTAX, "Invalid character in an object.");
		}
		if (parser->errorCode != ERROR_NONE) {
			break;
		}
	}

	return o;
}

static Array* parseArray(JSONParser *parser) {
	eatSpace(parser);

	char ch = pop(parser);

	FAIL(ch == 0, parser, ERROR_SYNTAX, "Premature end of documnent while parsing an array.");
	FAIL(ch != '[', parser, ERROR_SYNTAX, "JSON array does not start with '['.");

	Array *a = newArray(25);

	while ((ch = pop(parser)) != ']') {
		if (ch == 0) {
			deleteArray(a);
			a = NULL;
			save_error(parser, ERROR_SYNTAX, 
				"Premature end of documnent while parsing an array.");
			//Stop parsing array
			break;
		}
		
		putback(parser);

		JSONObject *o = parseValue(parser);
		if (o != NULL) {
			arrayAdd(a, o);
		} 
		eatSpace(parser);
		//Next character must be ',' or ']'
		ch = pop(parser);
		if (ch != ',' && ch != ']') {
			deleteArray(a);
			a = NULL;
			save_error(parser, ERROR_SYNTAX, "Invalid character in array.");
			//Stop parsing array
			break;
		}
		if (ch != ',') {
			putback(parser);
		}
	}

	return a;
}

static JSONObject* parseValue(JSONParser *parser) {
	JSONObject *o = NULL;

	eatSpace(parser);

	char ch = peek(parser);

	FAIL(ch == 0, parser, ERROR_SYNTAX, "Premature end of JSON string.");

	if (ch == '"') {
		String *str = parseString(parser);

		o = newJSONObject(JSON_STRING);
		o->value.string = str;
	} else if (ch == '{') {
		o = parseObject(parser);
	} else if (ch == '[') {
		Array *a = parseArray(parser);

		o = newJSONObject(JSON_ARRAY);
		o->value.array = a;
	} else if (isdigit(ch) || ch == '-') {
		o = newJSONObject(JSON_NUMBER);
		o->value.number = parseNumber(parser);
	} else if (ch == 't') {
		o = newJSONObject(JSON_BOOLEAN);
		o->value.booleanValue = parseBool(parser);
	} else if (ch == 'f') {
		o = newJSONObject(JSON_BOOLEAN);
		o->value.booleanValue = parseBool(parser);
	} else if (ch == 'n') {
		o = newJSONObject(JSON_NULL);
		o->value.isNull = parseNull(parser);
	}

	onValueParsed(parser, o);

	return o;
}

JSONObject *jsonParseCString(JSONParser *parser, const char *stringToParse) {
	String *str = newStringWithCString(stringToParse);
	JSONObject *o = jsonParse(parser, str);

	deleteString(str);

	return o;
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
	} else {
		save_error(parser, ERROR_SYNTAX, "Document does not start with '{' or '['.");
	}

	return parser->root;
}
