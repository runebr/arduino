#include <TinyDebugKnockBang.h>

void setup( void )
{
  Debug.begin( 250000 );
}

void loop( void )
{
  Debug.println( F( "Caitlin! " ) );
  delay( 1000 );
}
