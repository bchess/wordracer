#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <string>
#include <assert.h>

#define kWordLength 16
#define kBoardBits 4
#define kBoardSize (1 << kBoardBits) 

#define TOLOWER( x ) ( (x) >= 'A' && (x) <= 'Z' ? (x) + ('a'-'A') : (x) )

unsigned long stringSignature( const char * str )
{
  unsigned long signature = 0;
  
  while( (*str != 0x00 ) ) {
    signature = signature | ( 1 << ( TOLOWER(*str) - 'a' ) );
    str++;
  }
  return signature;
}

struct Word
{
  Word() : signature( 0 ) {}

  void set( const char * word, unsigned int length )
  {
    memcpy( text, word, length + 1 );
    signature = stringSignature( text );
  }

  unsigned long signature;
  unsigned int length;
  char text[ kWordLength ];
};

struct Board
{
  Board() : signature( 0 ), row( 0 ) {
    memset( lengths, 0, sizeof(lengths) );
    clearInUse();
  }

  void clearInUse()
  {
    memset( inUse, 0, sizeof( inUse ) );
  }

  void addLine( const char * text )
  {
    signature |= stringSignature( text );
    int i = 0;
    while( (*text) != 0x00 ) {
      if( (*text) != ' ' ) {
        char letter = TOLOWER(*text)-'a';
        locations[ letter ][ lengths[ letter ]++ ] = row * kBoardSize + i;
      }
      ++text;
      ++i;
    }
    ++row;
  }
  unsigned long signature;
  int locations[26][8];
  int lengths[26];
  bool inUse[kBoardSize*kBoardSize];
  int row;
};

bool tryWord( const char * text, int lengthCursor );

Board gBoard;

struct StrLenComparator : public std::binary_function< Word const &, Word const &, bool >{
  bool operator()( Word const & LHS, Word const & RHS ) const
  {
    return LHS.length > RHS.length;
  }
};

int main( int argc, char ** argv )
{
  std::vector< Word > wordList;

  if( argc != 2 ) {
    fprintf( stderr, "%s: <wordlist>\n", argv[0] );
    exit(1);
  }

  FILE *fp = fopen( argv[1], "r" );
  assert( fp != NULL );

  // Generate word list
  wordList.reserve(500000);
  char text[ kWordLength ];
  while( fgets( text, kWordLength, fp ) ) {
    // verify first
    int textLen = strlen( text );
    int i;
    text[ --textLen ] = 0x00; //remove endl
    for( i = 0 ; i < textLen; ++i ) {
      if( !( ( text[i] >= 'A' && text[i] <= 'Z' )  || ( text[i] >= 'a' && text[i] <= 'z' ) ) ) {
        break;
      }
    }
    if( i == textLen && textLen > 2 ) { 
      wordList.push_back( Word() );
      wordList.back().set( text, textLen );
    }
  }
  fclose( fp );

  std::sort( wordList.begin(), wordList.end(), StrLenComparator() );

  // Read board
  printf("Enter board\n");

  while( fgets( text, kWordLength, stdin ) ) {
    int textLen = strlen( text );
    if( textLen == 1 ) {
      break;
    } 
    text[ --textLen ] = 0x00; //remove endl
    gBoard.addLine( text );
  }

  // Now solve
  std::vector< Word >::iterator begin = wordList.begin();
  std::vector< Word >::iterator end = wordList.end();
  std::vector< Word >::iterator i;

  for( i = begin; i != end; ++i ) {
    if( ( (*i).signature | gBoard.signature ) == gBoard.signature ) {
      // Check this word more
      char * text = (*i).text;
      //printf("%s\n", text );
      if( tryWord( text, 0 ) ) {
        printf("%s\n", text );
        gBoard.clearInUse();
      }
    }
  }
}

bool tryWord( const char * text, int lengthCursor )
{
  char letterIdx = TOLOWER(*text) - 'a';
  if( *(++text) == 0x00 ) {
    return true;
  }
  if( gBoard.lengths[ letterIdx ] == lengthCursor ) {
    return false;
  }
  char nextLetterIdx = TOLOWER(*text) - 'a';
  if( letterIdx == 'q' - 'a' && nextLetterIdx == 'u' - 'a' ) {
    return tryWord( ++text, 0 ); // qu is always allowed
  }

  int loc = gBoard.locations[ letterIdx ][ lengthCursor ];

  gBoard.inUse[ loc ] = true;

  for( int a=0; a < gBoard.lengths[ nextLetterIdx ]; ++a ) {

    int nextLoc = gBoard.locations[ nextLetterIdx ][ a ];
    if( nextLoc == loc + kBoardSize || nextLoc == loc - kBoardSize || nextLoc == loc + 1 || nextLoc == loc - 1 ||
        nextLoc == loc + kBoardSize - 1 || nextLoc == loc + kBoardSize + 1 ||
        nextLoc == loc - kBoardSize - 1 || nextLoc == loc - kBoardSize + 1 ) {
      if( gBoard.inUse[ nextLoc ] == false && tryWord( text, a ) ) {
        return true;
      }
    }
  }
  gBoard.inUse[ loc ] = false;
  return false;
}
   
