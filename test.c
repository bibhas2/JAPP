#include <stdio.h>

#include "Parser.h"

int main() {
	String *str = newStringWithCString(
		"{\"array\":[10.5533E-2, \"a2\", 22.89, true , false , null ], \"num\": 11.23}"
		);
	JSONParser *p = newJSONParser();
	JSONObject *o = jsonParse(p, str);

	if (o == NULL) {
		puts("Parsing failed.");
		return 1;
	}
	double d = jsonGetNumber(o, "num");
	printf("Number %g\n", d);

	JSONObject *a = jsonGetArray(o, "array");

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
