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

int askMainMenuQuestions();
int askFileProcessQuestions();
struct movie* processMovieFile(char* filePath);
void largestFile();
int is_valid_movie_csv(const char *filename);

struct movie{
    char* name;
    int year;
    char* languages[5];
    double rating;
    struct movie* next;
};

int main (){

    int answer = askMainMenuQuestions();
    while (answer == 1) {
        int fileProcessAnswer = askFileProcessQuestions();

        // if 1 process largest file with csv extension whose name starts with prefix movies_
        if (fileProcessAnswer == 1) {
            largestFile();
            int answer = askMainMenuQuestions();
        }
        else {
            break;
        }

        // if 2 find smllest file with csv extension whose name starts with prefix movies_


        // if 3 ask user to enter name of a file
        // checks if file exists, if none, write error and provide choices again
        // no req for prefix or extension

    }

    // struct movie* head = processMovieFile(argv[1]);

    return 0;
};

int askFileProcessQuestions() {
    int answer;
    printf("Which file you want to process?\n");
    printf("Enter 1 to pick the largest file\n");
    printf("Enter 2 to pick the smallest file\n");
    printf("Enter 3 to specify the name of a file\n\n");
    printf("Enter a choice from 1 to 3: ");
    scanf("%d", &answer);
    while (answer < 1 || answer > 3) {
            printf("You entered an incorrect choice. Try again.\n");
            answer = askFileProcessQuestions();
        }
    return answer;
}


int askMainMenuQuestions() {
    int answer;
    printf("1. Select file to process\n");
    printf("2. Exit the program\n\n");
    printf("Enter a choice 1 or 2: ");
    scanf("%d", &answer);
    while (answer < 1 || answer > 2) {
        printf("You entered an incorrect choice. Try again.\n");
        answer = askMainMenuQuestions();
    }
    return answer;
}

void largestFile() {
    struct dirent *entry;
    struct stat fileStat;

    // open the current directory
    DIR *dir = opendir(".");

    char largestFile[256] = "";
    off_t largestSize = 0;

    while ((entry = readdir(dir)) != NULL) {
        // skip dirs and make sure .csv
        if (entry->d_type != DT_DIR && is_valid_movie_csv(entry->d_name)) {
            // get file stats
            if (stat(entry->d_name, &fileStat) == 0) {
                if (fileStat.st_size > largestSize) {
                    largestSize = fileStat.st_size;
                    strncpy(largestFile, entry->d_name, sizeof(largestFile) - 1);
                    largestFile[sizeof(largestFile) - 1] = '\0';
                }
            }
        }
    }

    closedir(dir);

    printf("Now processing the chosen file named %s\n", largestFile);
    struct movie* head = processMovieFile(largestFile);

    int random_number = rand() % 100000;
    char dirName[300] = "bergerst.movies.";
    // append random number to directory name
    sprintf(dirName + strlen(dirName), "%d", random_number);
    
    // make directory wit rwxr-x--- permissions
    mkdir(dirName, 0750);

    printf("Created directory with name %s.\n\n", dirName);

    // create a file inside the directory for each year a movie was released
    while (head != NULL) {
        char filePath[350];
        snprintf(filePath, sizeof(filePath), "%s/%d.txt", dirName, head->year);

        // create file with 0640 permissions
        int fd = open(filePath, O_WRONLY | O_CREAT | O_APPEND, 0640);

        FILE *file = fdopen(fd, "a");

        fprintf(file, "%s\n", head->name);
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
struct movie* processMovieFile(char* filePath){
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
    // printf("\nProcessed file %s and parsed data for %d movies\n", filePath, movies);
    return head;
};


