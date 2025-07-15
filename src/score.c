/* ---------------------------------------------------
 * score.C
 * ---------------------------------------------------
 * GRUPO 1:
 * 	CASTRO, Tomás
 *	FRIGERIO, Dylan
 * 	VALENZUELA, Agustín
 * 	YAGGI, Lucca
 *
 * Profesores:
 * 	MAGLIOLA, Nicolas
 * 	JACOBY, Daniel
 * 	VACATELLO, Pablo
 *
 * fecha: 15/07/2025
 * ---------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "score.h"

#define MAX_CANT_SCORES 100

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
    file = fopen("data/topScore.txt", "w");
    if (!file) return -1; // Error opening file for writing


    for (int i = 0; i < count; i++) {
        fprintf(file, "%s %d\n", topScore[i].name, topScore[i].score);
    }
    fclose(file);
    return 1; // Operation successful
}