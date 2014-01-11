#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "parse_dms.h"

int main() {	
	FILE* infile = fopen("coords", "r");
	if (infile == NULL) {
		printf("failed to open input file.\n");
		exit(1);
	}	

	double* lon = malloc(sizeof(double));
	double* lat = malloc(sizeof(double));
	char* line = NULL;		
	size_t nbytes;
	dms_parser_state result;

	while (getline(&line, &nbytes, infile) != -1) {
		if (line[0] != ';') {
			printf("raw: %s ", line);
			result = parse_dms(line, lat, lon);
			switch (result) {
				case SUCCESS: printf("lat:%f  lon:%f\n", *lat, *lon);
					      break;
				case INVALID_NUMBER_NUMERIC_COMPONENTS: printf("invalid # numeric components.\n"); break;


	    			case NUMERIC_PARSE_ERROR: printf("numeric parse error\n"); break;
              			case TOO_MANY_CARDINAL_DIRECTIONS: printf("too many cardinal directions\n");  break;
				case COORDINATES_NOT_SAME_FORMAT: printf("coordinates not same format\n"); break;
				default: printf("unspecified parse error\n");
			}
		}		
		free(line);
		line = NULL;
	}	

	free(line);
	free(lon);
	free(lat);
	fclose(infile);
	return 0;
}

