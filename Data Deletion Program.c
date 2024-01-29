#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to generate secure random data
void secureRandomData(char *buffer, size_t size) {
    FILE *random = fopen("/dev/urandom", "r");
    if (random == NULL) {
        perror("Error opening /dev/urandom");
        exit(1);
    }

    fread(buffer, sizeof(char), size, random);
    fclose(random);
}

// Function to overwrite file with specific patterns (DoD 5220.22-M standard)
void overwriteFile(const char *filename, size_t size) {
    char *buffer = malloc(size);
    if (buffer == NULL) {
        perror("Memory allocation error");
        exit(1);
    }

    FILE *file;
    const char patterns[3] = {0x00, 0xFF, 0x00}; // Patterns: 0, 1, and random

    for (int i = 0; i < 3; ++i) { // Three passes
        if (i == 2) { // Random data for the third pass
            secureRandomData(buffer, size);
        } else {
            memset(buffer, patterns[i], size);
        }

        file = fopen(filename, "w");
        if (file == NULL) {
            perror("Error opening file");
            free(buffer);
            exit(1);
        }
        fwrite(buffer, sizeof(char), size, file);
        fclose(file);
    }

    free(buffer);
}

// File deletion confirmation
int confirmDeletion() {
    printf("Are you sure you want to permanently delete this file? (y/n): ");
    char response;
    scanf(" %c", &response);
    return (response == 'y' || response == 'Y');
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    printf("Selected file for deletion: %s\n", filename);

    if (!confirmDeletion()) {
        printf("Deletion canceled.\n");
        return 0;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fclose(file);

    overwriteFile(filename, size); // Overwrite file with DoD 5220.22-M standard

    // Rename file to obscure original name
    char tempName[] = "tempXXXXXX";
    int result = rename(filename, tempName);
    if (result != 0) {
        perror("Error renaming file");
        return 1;
    }

    // Delete the file
    if (remove(tempName) != 0) {
        perror("Error deleting file");
        return 1;
    }

    printf("File securely deleted.\n");
    return 0;
}
