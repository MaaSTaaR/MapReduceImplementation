int isResult( char **items, int numberOfItems, char *line );
int searchInFile( char **items, int numberOfItems, FILE *targetFile, void ( *printHandler )( char *message, int connection ), int connection );
