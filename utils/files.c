#include <stdbool.h>
#include <stdio.h>

bool files_equal(FILE *file1, FILE *file2) {
	char buff1[100];
	char buff2[100];
	while (true) {
		char *finish1 = fgets(buff1, 100, file1);
		char *finish2 = fgets(buff2, 100, file2);
		if (finish1 == NULL && finish2 == NULL) {
			return true;
		}
		if ((finish1 == NULL && finish2 != NULL) ||
		    (finish1 != NULL && finish2 == NULL)) {
			return false;
		}
		for (int i = 0; i < 100; i++) {
			if (buff1[i] == 0 && buff2[i] == 0) {
				break;
			}
			if ((buff1[i] == 0 && buff2[i] != 0) ||
			    (buff1[i] != 0 && buff2[i] == 0)) {
				return false;
			}
			if (buff1[i] != buff2[i]) {
				return false;
			}
		}
	}
}
