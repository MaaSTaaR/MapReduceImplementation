void clearCRLF( char *string );
void sendMessage( int, char * );
void createServer( void ( *requestHandler )( int connection ), int port );

