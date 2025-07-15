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
 * 	MAGLIOLA, Nicolas
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

/* ---------------------------------------------------
 * @brief Loads the top scores from the file for the given platform.
 * 
 * Reads up to `cantScores` scores into the provided array `topScore`.
 * Returns the number of scores read or -1 if the file could not be opened.
 * 
 * @param topScore Pointer to array where scores will be stored.
 * @param cantScores Maximum number of scores to read.
 * @return Number of scores read, or -1 on file open failure.
 * ---------------------------------------------------*/
int getTopScore(score_t *topScore, int cantScores);


/* ---------------------------------------------------
 * @brief Updates the top scores list with a new player score.
 * 
 * Loads existing top scores from file, checks for duplicates,
 * inserts the new score in sorted order if it qualifies,
 * and writes the updated list back to the file (max 10 entries).
 * 
 * @param newScore New score to consider for top list.
 * @param name Player name associated with new score.
 * @return 1 if updated successfully,
 *         0 if duplicate entry not added,
 *        -1 if file could not be opened.
 * ---------------------------------------------------*/
int topScoreUpdate(int newScore, const char *name);

#endif // SCORE_H
