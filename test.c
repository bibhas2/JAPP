#include <stdio.h>

#include "Parser.h"

int main() {
	String *str = newStringWithCString(
		"{\"array\":[10.5533E-2, \"a2\", 22.89], \"num\": 11.23}"
		);
	JSONParser *p = newJSONParser();
	JSONObject *o = jsonParse(p, str);

	if (o == NULL) {
		puts("Parsing failed.");
		return 1;
	}
	double d = jsonGetNumber(o, "num");
	printf("Number %g\n", d);

	deleteJSONParser(p);

	return 0;
}
