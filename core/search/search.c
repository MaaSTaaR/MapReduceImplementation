#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int getPosition( char *string, char *word )
{
	char *wordAddressInString = strstr( string, word );
	int position = 0;
	
	while ( wordAddressInString != string && *string != '\0' )
	{
		position++;
		string++;
	}
	
	if ( *string == '\0' )
		position = -1;
	
	return position;
}

int isResult( char **items, int numberOfItems, char *line )
{
	size_t bufsize = 0;
	
	int itemIdx = 0, numberOfItemsInLine = 0, prevItemPosition = -1;
	int considerAsResult = 1;
	int wordExistInLine[ numberOfItems ];
	
	// ... //
	
	while ( itemIdx < numberOfItems )
	{
		char *currItem = items[ itemIdx ];
		
		if ( strstr( line, currItem ) != NULL )
		{
			numberOfItemsInLine++;
			
			if ( prevItemPosition != -1 )
			{
				// Violation of order rule
				if ( getPosition( line, currItem ) < prevItemPosition )
				{
					considerAsResult = 0;
					
					return 0;
				}
			}
			
			prevItemPosition = getPosition( line, currItem );
			wordExistInLine[ itemIdx ] = 1;
		}
		else
		{
			wordExistInLine[ itemIdx ] = 0;
		}
		
		itemIdx++;
	}
	
	// ... //
	
	if ( numberOfItems > 1 )
		if ( wordExistInLine[ 0 ] == 0 || wordExistInLine[ 1 ] == 0 )
			return 0;
	
	// ... //
	
	// Violation
	if ( considerAsResult == 0 )
		return 0;
	
	// Not a result at all
	if ( numberOfItemsInLine == 0 )
		return 0;
		
	// ... //
	
	if ( numberOfItems > 1 && numberOfItemsInLine < 2 )
		return 0;
	
	// ... //
	
	// And we got a result.
	
	/*int resultSize = ( strlen( line ) * ( sizeof( char ) * 2 ) ) + 20;
	char *result = malloc( resultSize );
	
	memset( result, 0, resultSize );
	sprintf( result, "    Result = %s", line );
	printHandler( result, connection );*/
	
	return 1;
}

int searchInFile( char **items, int numberOfItems, FILE *targetFile, void ( *printHandler )( char *message, int connection ), int connection )
{
	char *line = NULL, toBePrinttedMessage[ 10000 ];
	size_t bufsize = 0;
	
	int s = 0, resultsNumber = 0;
	
	while ( getline( &line, &bufsize, targetFile ) != -1 )
	{
		//if ( s++ == 15 )
		//	break;
		
		int itemIdx = 0, numberOfItemsInLine = 0, prevItemPosition = -1;
		int considerAsResult = 1;
		int wordExistInLine[ numberOfItems ];
		
		// ... //
		
		while ( itemIdx < numberOfItems )
		{
			char *currItem = items[ itemIdx ];
			
			if ( strstr( line, currItem ) != NULL )
			{
				numberOfItemsInLine++;
				
				if ( prevItemPosition != -1 )
				{
					// Violation of order rule
					if ( getPosition( line, currItem ) < prevItemPosition )
					{
						considerAsResult = 0;
						break;
					}
				}
				
				prevItemPosition = getPosition( line, currItem );
				wordExistInLine[ itemIdx ] = 1;
			}
			else
			{
				wordExistInLine[ itemIdx ] = 0;
			}
			
			itemIdx++;
		}
		
		// ... //
		
		if ( numberOfItems > 1 )
			if ( wordExistInLine[ 0 ] == 0 || wordExistInLine[ 1 ] == 0 )
				continue;
		
		// ... //
		
		// Violation
		if ( considerAsResult == 0 )
			continue;
		
		// Not a result at all
		if ( numberOfItemsInLine == 0 )
			continue;
			
		// ... //
		
		if ( numberOfItems > 1 && numberOfItemsInLine < 2 )
			continue;
		
		// ... //
		
		// And we got a result.
		
		/*int resultSize = ( strlen( line ) * ( sizeof( char ) * 2 ) ) + 20;
		char *result = malloc( resultSize );
		
		memset( result, 0, resultSize );
		sprintf( result, "    Result = %s", line );
		printHandler( result, connection );*/
		
		resultsNumber++;
	}
	
	//memset( toBePrinttedMessage, 0, 10000 );
	//sprintf( toBePrinttedMessage, "\n    # of result in this file = %d\n", resultsNumber );
	//printHandler( toBePrinttedMessage, connection );
	
	fclose( targetFile );
	
	return resultsNumber;
}
