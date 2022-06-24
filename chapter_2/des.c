#define GET_BIT( array, bit ) \
    ( array[ ( int ) ( bit / 8 ) ] & ( 0x8 >> ( bit % 8 ) ) )

#define CLEAR_BIT( array, bit ) \
    ( array[ ( int ) ( bit / 8 ) ] &= ~( 0x8 >> ( bit % 8 ) ) )
