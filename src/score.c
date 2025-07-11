#include <stdio.h>
#include <string.h>

#include "score.h"

#define MAX_CANT_SCORES 100

// int main(void)
// {
//     score_t topScores[10] = {
//         {"juan", 1000},
//         {"maria", 900},
//         {"pedro", 800},
//         {"luisa", 700},
//         {"carlos", 600},
//         {"sofia", 500},
//         {"jose", 400},
//         {"ana", 300},
//         {"luis", 200},
//         {"marta", 100}
//     };
//     // Update the top scores with a new score
//     int newScore = 950;
//     const char *name = "nuevo_jugador";
//     int result = topScoreUpdate(newScore, name);
//     if (result == 1) {
//         printf("Score updated successfully.\n");
//     } else if (result == 0) {
//         printf("Duplicate score and name, not added.\n");
//     } else {
//         printf("Error updating score.\n");
//         return 1;
//     }
//     // Retrieve the top scores
//     printf("Retrieving top scores...\n");
//     score_t topScores[MAX_CANT_SCORES];
//     int topCount = 10; // Assuming we want the top 10 scores
//     // Get the top scores from the file
//     int count = getTopScore(topScores, MAX_CANT_SCORES);
    
//     if (count < 0) {
//         printf("Error reading scores.\n");
//         return 1;
//     }

//     printf("Top Scores:\n");
//     for (int i = 0; i < count; i++) {
//         printf("%s: %d\n", topScores[i].name, topScores[i].score);
//     }

//     return 0;
// }

int getTopScore(score_t *topScore, int cantScores)
{
    if(MAX_CANT_SCORES < cantScores)
    {
        return -1; // Invalid number of scores
    }
    // Open the file for reading
    FILE *file = fopen("data/topScore.txt", "r");
    if (!file) 
    {
        return -1; // Error opening file
    }

    // Read up to 10 scores from the file
    int count = 0;
    while (count < cantScores && fscanf(file, "%14s %d", topScore[count].name, &topScore[count].score) == 2) {
        count++;
    }
    fclose(file);
    
    return count; // Return the number of scores read
}

int topScoreUpdate(int newScore, const char *name)
{
    int count = 1;
    // Open the file for reading
    FILE *file = fopen("data/topScore.txt", "r");
    if (!file) // Error opening file
    {
        // Create the file if it doesn't exist
        file = fopen("data/topScore.txt", "w");
        if (!file) return -1; // Error opening file for writing
        fprintf(file, "%s %d\n", name, newScore);
        fclose(file);
        return 1;
    }

    
    // Read the current top scores from the file
    score_t topScore[MAX_CANT_SCORES];
    count = getTopScore(topScore, MAX_CANT_SCORES);
    if (count < 0) 
    {   
        count = 0; // If file does not exist or error, start with zero scores
    }

    // Check for exact duplicates (same score and name)
    for (int i = 0; i < count; i++) {
        if (topScore[i].score == newScore && strcmp(topScore[i].name, name) == 0) {
            return 0; // Duplicate found, do not add
        }
    }

    // Find the correct position to insert the new score (descending order)
    int insert_pos = -1;
    for (int i = 0; i < count; i++) {
        if (newScore > topScore[i].score) {
            insert_pos = i;
            break;
        }
    }
    // If the new score is not higher than any existing and there is space, add at the end
    if (insert_pos == -1 && count < 10) {
        insert_pos = count;
    }
    // If the score qualifies for the top 10, insert it and shift lower scores down
    if (insert_pos != -1) {
        for (int i = (count < 10 ? count : 9); i > insert_pos; i--) {
            topScore[i] = topScore[i-1]; // Shift scores down to make space
        }
        // Insert the new score and name
        topScore[insert_pos].score = newScore;
        strncpy(topScore[insert_pos].name, name, sizeof(topScore[insert_pos].name)-1);
        topScore[insert_pos].name[sizeof(topScore[insert_pos].name)-1] = '\0'; // Ensure null-termination
        if (count < MAX_CANT_SCORES)
            count++; // Increase count if there is space
    }
    
    // Write the updated top scores back to the file
    file = fopen("../data/topScore.txt", "w");
    if (!file) return -1; // Error opening file for writing


    for (int i = 0; i < count; i++) {
        fprintf(file, "%s %d\n", topScore[i].name, topScore[i].score);
    }
    fclose(file);
    return 1; // Operation successful
}