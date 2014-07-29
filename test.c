#include <stdio.h>

#include "Parser.h"

int main() {
	char *cStr = "{\"array\":[10.5533E-2, \"I \\u2665 Miami\", 22.89, true , false , null ], \"num\": 11.23}";
	String *str = newStringWithCString(cStr);
	puts(stringAsCString(str));
	JSONParser *p = newJSONParser();
	JSONObject *o = jsonParse(p, str);

	if (o == NULL) {
		puts("Parsing failed.");
		return 1;
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


	//Test C string API
	o = jsonParseCString(p, cStr);
	a = jsonGetArray(o, "array");

	printf("String is %s\n", jsonGetCStringAt(a, 1));
	
	deleteJSONParser(p);

	return 0;
}
