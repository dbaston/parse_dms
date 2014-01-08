#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

typedef enum {SUCCESS,
	      INVALID_NUMBER_NUMERIC_COMPONENTS,
	      NUMERIC_PARSE_ERROR,
              TOO_MANY_CARDINAL_DIRECTIONS} dms_parser_state;

dms_parser_state parse_dms(const char*, double*, double*);

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
			   
dms_parser_state parse_dms(const char* orig_input, double* lat, double* lon) {
	typedef enum {NUMERIC, NON_NUMERIC} state;
	char* input = strdup(orig_input);
	char cur = input[0];
	char* num_start = NULL; /* start point of a substring to extract */
	
	/* Use these to hold cardinal directions as they are encountered. */
	char first_cardinal = 'A';
	char second_cardinal = 'A';	
	double numeric_components[6] = {0};

	double coord1 = 0;
	double coord2 = 0;

	state cur_state = NON_NUMERIC;
	short component_idx = -1;
	size_t i;
	
	for (i = 0; cur != '\0'; i++) {
		cur = input[i];
				
		if (isdigit(cur) || cur == '.' || (cur == '-' && cur_state == NON_NUMERIC)) {
			if (cur_state == NON_NUMERIC) {
				/* Start of a number */
				component_idx++;
				if (component_idx > 5) {
					free(input);
					return INVALID_NUMBER_NUMERIC_COMPONENTS;
				}				
				cur_state = NUMERIC;
				num_start = &input[i];
			}
		} else {
			if (cur == 'N' || cur == 'S' || cur == 'E' || cur == 'W') {
				if (first_cardinal == 'A') {
					first_cardinal = cur;
				} else if (second_cardinal == 'A') {
					second_cardinal = cur;
				} else {
					free(input);
					return TOO_MANY_CARDINAL_DIRECTIONS;
				}
			}

			if (cur_state == NUMERIC)
			{
				/* End of a number */
				cur_state = NON_NUMERIC;
				input[i] = '\0';
			
				/* Attempt to convert substring to double */
				char* endptr;
				numeric_components[component_idx] = strtod(num_start, &endptr);
				if (endptr != NULL && endptr[0] != '\0') {
					printf("Converting %s to float, got %f with endptr:  >%s<\n", num_start, numeric_components[component_idx], endptr);
					return NUMERIC_PARSE_ERROR;		
				}
			}
		}		
	}		

	/* Interpret the meaning of the numeric elements according to the total number of
	   numeric elements. */
	switch (component_idx+1) {
		case 6: coord1 = (numeric_components[0] < 0 ? -1 : 1)*(abs(numeric_components[0]) + (numeric_components[1] / 60.0) + (numeric_components[2] / 3600.0));
			coord2 = (numeric_components[3] < 0 ? -1 : 1)*(abs(numeric_components[3]) + (numeric_components[4] / 60.0) + (numeric_components[5] / 3600.0));
			break;
		case 4: coord1 = (numeric_components[0] < 0 ? -1 : 1)*(abs(numeric_components[0]) + (numeric_components[1]/60.0));
			coord2 = (numeric_components[2] < 0 ? -1 : 1)*(abs(numeric_components[2]) + (numeric_components[3] / 60.0));
			break;
		case 2: coord1 = numeric_components[0];
			coord2 = numeric_components[1];			
			break;
		default: free(input);
			 return INVALID_NUMBER_NUMERIC_COMPONENTS; 
	}
	
	/* Inspect our cardinals.  Do they tell us the order of the components?
	   If not, assume lat-lon. */
	if (first_cardinal == 'E' || first_cardinal == 'W') {
		/* lon-lat */
		*lon = (coord1 > 0 && first_cardinal  == 'W' ? -1*coord1 : coord1);
		*lat = (coord2 > 0 && second_cardinal == 'S' ? -1*coord2 : coord2);
	} else {
		*lat = (coord1 > 0 && first_cardinal  == 'S' ? -1*coord1 : coord1);
		*lon = (coord2 > 0 && second_cardinal == 'W' ? -1*coord2 : coord2);
	}
		
	free(input);
	return SUCCESS;
}
 
