#include <stdio.h>

#include "Parser.h"

String *loadFile(const char *file) {
	FILE *f = fopen(file, "r");

	if (f == NULL) {
		return NULL;
	}

	String *s = newStringWithCapacity(1024);
	char buffer[256];
	size_t sz;

	while ((sz = fread(buffer, 1, sizeof(buffer), f)) > 0) {
		stringAppendBuffer(s, buffer, sz);
		if (sz < sizeof(buffer)) {
			break;
		}
	}

	fclose(f);

	return s;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		puts("Usage: test json_file");
		return 1;
	}
	String *str = loadFile(argv[1]);
	if (str == NULL) {
		puts("Could not load input JSON file.");
		return 2;
	}

	JSONParser *p = newJSONParser();
	JSONObject *o = jsonParse(p, str);

	if (p->errorCode != ERROR_NONE) {
		printf("Parsing failed at line: %d. Message: %s\n",
			p->errorLine, p->errorMessage);
		return 3;
	}
	double d = jsonGetNumber(o, "num");
	printf("Number %g\n", d);

	JSONObject *a = jsonGetArray(o, "array");

	String *s = jsonGetStringAt(a, 1);
	printf("String is: %s\n",
		stringAsCString(jsonGetStringAt(a, 1)));
	printf("Bool is: %s\n",
		jsonGetBooleanAt(a, 3) ? "true" : "false");
	printf("Bool is: %s\n",
		jsonGetBooleanAt(a, 4) ? "true" : "false");
	printf("Is null: %s\n",
		jsonIsNullAt(a, 5) == true ? "true" : "false");
	printf("Is null: %s\n",
		jsonIsNullAt(a, 1) == true ? "true" : "false");

	deleteJSONParser(p);

	return 0;
}
