#include <stdbool.h>
#include <stdio.h>

bool files_equal(FILE *file1, FILE *file2) {
	int buff_size = 100;
	char buff1[buff_size];
	char buff2[buff_size];
	while (true) {
		char *finish1 = fgets(buff1, buff_size, file1);
		char *finish2 = fgets(buff2, buff_size, file2);
		if (finish1 == NULL && finish2 == NULL) {
			return true;
		}
		if ((finish1 == NULL && finish2 != NULL) ||
		    (finish1 != NULL && finish2 == NULL)) {
			return false;
		}
		for (int i = 0; i < buff_size; i++) {
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
