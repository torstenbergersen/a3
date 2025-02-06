/*
Program name: bergest_assignment3.c
Author: Torsten Bergersen
Program reads directory entries, finds a file in current directory based on user specifications, reads and processes chosen file, creates a new directory, and finally writes proessed data to new files in the new directory.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

int ask_main_menu_questions();
int ask_file_process_questions();
struct movie* process_movie_file(const char* filePath);
void largest_file();
void smallest_file();
void file_by_name();
int is_valid_movie_csv(const char *filename);
void create_dir_and_files(const char *filename);

struct movie{
    char* name;
    int year;
    char* languages[5];
    double rating;
    struct movie* next;
};

int main (){

    int answer = ask_main_menu_questions();
    while (answer == 1) {
        int fileProcessAnswer = ask_file_process_questions();

        // if 1 process largest file with csv extension whose name starts with prefix movies_
        if (fileProcessAnswer == 1) {
            largest_file();
        }
        // if 2 find smllest file with csv extension whose name starts with prefix movies_
        else if (fileProcessAnswer == 2) {
            smallest_file();
        }
        // if 3 ask user to enter name of a file
        // checks if file exists, if none, write error and provide choices again
        // no req for prefix or extension
        else if (fileProcessAnswer == 3) {
            file_by_name();
        }
        answer = ask_main_menu_questions();
    }

    return 0;
};

int ask_file_process_questions() {
    int answer;
    printf("\nWhich file you want to process?\n");
    printf("Enter 1 to pick the largest file\n");
    printf("Enter 2 to pick the smallest file\n");
    printf("Enter 3 to specify the name of a file\n\n");
    printf("Enter a choice from 1 to 3: ");
    scanf("%d", &answer);
    while (answer < 1 || answer > 3) {
            printf("You entered an incorrect choice. Try again.\n");
            answer = ask_file_process_questions();
        }
    return answer;
}


int ask_main_menu_questions() {
    int answer;
    printf("1. Select file to process\n");
    printf("2. Exit the program\n\n");
    printf("Enter a choice 1 or 2: ");
    scanf("%d", &answer);
    while (answer < 1 || answer > 2) {
        printf("You entered an incorrect choice. Try again.\n");
        answer = ask_main_menu_questions();
    }
    return answer;
}

void largest_file() {
    struct dirent *entry;
    struct stat fileStat;

    // open the current directory
    DIR *dir = opendir(".");

    char largestFile[256] = "";
    // to store file size
    off_t largestSize = 0;

    while ((entry = readdir(dir)) != NULL) {
        // skip dirs and make sure .csv
        if (entry->d_type != DT_DIR && is_valid_movie_csv(entry->d_name)) {
            // get file stats and identify largest file
            if (stat(entry->d_name, &fileStat) == 0) {
                if (fileStat.st_size > largestSize) {
                    largestSize = fileStat.st_size;
                    strncpy(largestFile, entry->d_name, sizeof(largestFile) - 1);
                    // manually null-terminate
                    largestFile[sizeof(largestFile) - 1] = '\0';
                }
            }
        }
    }

    closedir(dir);
    create_dir_and_files(largestFile);

}

void smallest_file() {
    struct dirent *entry;
    struct stat fileStat;

    // open the current directory
    DIR *dir = opendir(".");

    char smallestFile[256] = "";
    // to store file size
    off_t smallestSize = -1;

    while ((entry = readdir(dir)) != NULL) {
        // skip dirs and make sure .csv
        if (entry->d_type != DT_DIR && is_valid_movie_csv(entry->d_name)) {
            // get file stats and identify smallest file
            if (smallestSize == -1 || fileStat.st_size < smallestSize) {
                smallestSize = fileStat.st_size;
                strncpy(smallestFile, entry->d_name, sizeof(smallestFile) - 1);
                // manually null-terminate
                smallestFile[sizeof(smallestFile) - 1] = '\0';
            }
        }
    }

    closedir(dir);
    create_dir_and_files(smallestFile);

}

void file_by_name() {
    struct dirent *entry;
    struct stat fileStat;

    // open the current directory
    DIR *dir = opendir(".");

    char filename[256];
    printf("Enter the complete file name: ");
    scanf("%s", filename);

    // found file flag
    int file_found = 0;

    while ((entry = readdir(dir)) != NULL) {
        // compare current entry with input filename
        if (strcmp(entry->d_name, filename) == 0) {
            file_found = 1;
            break; 
        }
    }

    closedir(dir);

    if (!file_found) {
        printf("The file %s was not found. Try again\n", filename);
        return;
    }

    // if a valid file is found, process it
    create_dir_and_files(filename);
}

void create_dir_and_files(const char *filename) {

    printf("Now processing the chosen file named %s\n", filename);
    struct movie* head = process_movie_file(filename);

    // random number 0-99999 inclusive
    int random_number = rand() % 100000;
    // dir name prefix
    char dirName[300] = "bergerst.movies.";
    // append random number to directory name
    sprintf(dirName + strlen(dirName), "%d", random_number);
    
    // make directory wit rwxr-x--- permissions
    mkdir(dirName, 0750);

    printf("Created directory with name %s\n\n", dirName);

    // create a file inside the directory for each year a movie was released
    while (head != NULL) {
        // string to hold file path
        char filePath[350];
        // create path
        snprintf(filePath, sizeof(filePath), "%s/%d.txt", dirName, head->year);

        // create file with 0640 permissions
        int fd = open(filePath, O_WRONLY | O_CREAT | O_APPEND, 0640);

        // open file to write names to
        FILE *file = fdopen(fd, "a");

        // write movie name to corresponding year file
        fprintf(file, "%s\n", head->name);
        // close file
        fclose(file);

        head = head->next;
    }
}



int is_valid_movie_csv(const char *filename) {
    size_t len = strlen(filename);
    size_t prefix_len = strlen("movies_");
    size_t ext_len = strlen(".csv");

    // ensure the filename ends with ".csv"
    if (strncmp(filename, "movies_", prefix_len) == 0 && strcmp(filename + (len - ext_len), ".csv") == 0) {
        return 1; 
    }
    return 0;
}


/* 
basic structure for reading csv from provided file for assigment (movies.c) 
*/ 
struct movie* process_movie_file(const char* filePath){
    char *currLine = NULL;
    size_t len = 0;

    // Open the specified file for reading only
    FILE *movieFile = fopen(filePath, "r");

    // amount of movies (-1 to account for title line)
    int movies = -1;

    // The head of the linked list
    struct movie* head = NULL;
    // The tail of the linked list
    struct movie* tail = NULL;

    // Read the file line by line
    while(getline(&currLine, &len, movieFile) != -1) {
        if (movies != -1) {
            struct movie* newMovie = malloc(sizeof(struct movie));
            if (newMovie == NULL) {
                printf("Memory allocation failed\n");
                exit(1);
            }

            // tokenizer for line parsing
            char* saveptr1;
            char* token = strtok_r(currLine, ",", &saveptr1);

            // store movie name
            newMovie->name = strdup(token);

            // store release year
            token = strtok_r(NULL, ",", &saveptr1);
            newMovie->year = atoi(token);

            // process languages
            token = strtok_r(NULL, ",", &saveptr1);
            char* saveptr2;
            char* langToken = strtok_r(token, ";", &saveptr2);
            int i = 0;
            while (langToken != NULL) {
                // remove leading or trailing '[ ]'
                if (langToken[0] == '[') {
                    langToken++; 
                }
                size_t len = strlen(langToken);
                if (len > 0 && langToken[len - 1] == ']') {
                    langToken[len - 1] = '\0';  // Replace ']' with null terminator
                }

                newMovie->languages[i] = strdup(langToken);
                i++;
                langToken = strtok_r(NULL, ";", &saveptr2);
            }
            // fill remaining slots with NULL
            for (; i < 5; i++) {
                newMovie->languages[i] = NULL;
            }

            // store rating
            token = strtok_r(NULL, ",", &saveptr1);
            newMovie->rating = atof(token);

            // append the movie to the linked list
            if(head == NULL){
                head = newMovie;
                tail = newMovie;
            } else{
                tail->next = newMovie;
                tail = newMovie;
            }
        }   
        movies++;
    }

    // Free the memory allocated by getline for currLine
    free(currLine);
    // Close the file
    fclose(movieFile);

    return head;
};


