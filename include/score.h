/* ---------------------------------------------------
 * score.h
 * ---------------------------------------------------
 * GRUPO 1:
 * 	CASTRO, Tomás
 *	FRIGERIO, Dylan
 * 	VALENZUELA, Agustín
 * 	YAGGI, Lucca
 *
 * Profesores:
 * 	MAGLIIOLA, Nicolas
 * 	JACOBY, Daniel
 * 	VACATELLO, Pablo
 *
 * fecha: 15/07/2025
 * ---------------------------------------------------*/

#ifndef SCORE_H
#define SCORE_H

enum{PC, PI};

typedef struct 
{
    int score;
    char name[15];
}score_t;

/**
 * @brief Loads the top scores from the corresponding file for the given platform.
 *
 * @param topScore Pointer to an array of score_t where the loaded scores will be stored.
 * @param platform Platform identifier (PC or PI) to select the correct file.
 * @return The number of scores read from the file, or -1 if the file could not be opened.
 */
int getTopScore(score_t *topScore, int platform);

/**
 * @brief Updates the top scores list with a new score and player name for the specified platform.
 *
 * This function reads the current top scores from a file, checks if the new score and name combination
 * already exists to avoid duplicates, and inserts the new score in the correct position if it qualifies.
 * The updated top scores are then written back to the file. The list is limited to the top 10 scores.
 *
 * @param newScore The new score to be considered for the top scores list.
 * @param name The name of the player associated with the new score.
 * @param platform The platform identifier (e.g., PC or another platform).
 * @return int Returns 1 if the operation was successful, 0 if the score was a duplicate and not added,
 *         or -1 if there was an error opening the file.
 */
int topScoreUpdate(int newScore, const char *name, int platform);

#endif // SCORE_H
