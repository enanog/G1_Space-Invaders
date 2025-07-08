#include <stdio.h>
#include <string.h>

#include "score.h"

int getTopScore(score_t *topScore, int platform)
{
    char fileName[30];
    // Select file name based on platform
    sprintf(fileName, "../data/topScoreP%c.txt", (platform == PC) ? 'C' : 'I');

    // Open the file for reading
    FILE *file = fopen(fileName, "r");
    if (!file) return -1; // Error opening file

    // Read up to 10 scores from the file
    int count = 0;
    while (count < 10 && fscanf(file, "%14s %d", topScore[count].name, &topScore[count].score) == 2) {
        count++;
    }
    fclose(file);
    
    return count; // Return the number of scores read
}

int topScoreUpdate(int newScore, const char *name, int platform)
{
    char fileName[30];
    // Select the file name based on the platform (PC or PI)
    sprintf(fileName, "../data/topScoreP%c.txt", (platform == PC) ? 'C' : 'I');

    // Read the current top scores from the file
    score_t topScore[10] = {0};
    int count = getTopScore(topScore, platform);
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
        if (count < 10) count++; // Increase count if there is space
    }

    // Write the updated top scores back to the file
    FILE *file = fopen(fileName, "w");
    if (!file) return -1; // Error opening file for writing
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s %d\n", topScore[i].name, topScore[i].score);
    }
    fclose(file);
    return 1; // Operation successful
}