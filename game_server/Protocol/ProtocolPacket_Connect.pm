###################################
# Packet base class               #
###################################

# format: 
# id       2
# index    4
# flags    2
# version  2
# game     string   

package ProtocolPacket_Connect;

use strict;

use Protocol::ProtocolPacket;
use Carp;


our @ISA = "ProtocolPacket";

# Reply codes

our $REP_CODE_WELCOME = 0;
our $REP_CODE_PROTOCOL_TO_OLD = 1;
our $REP_CODE_SERVER_FULL = 2;
our $REP_CODE_GAME_NOT_SERVED = 3;


###############################################
# Generate this packet                        #
###############################################
sub init {
   my $self = shift;

   $self->{VERSION_MINOR} = undef;
   $self->{VERSION_MAJOR} = undef;
   $self->{GAME} = undef;
}

################################################
# Read this packet, checks for correct syntax  #
################################################
sub read {
   my ( $self, $server, $client ) = @_;
 
   my $version = $client->read_16();
   $self->{VERSION_MAJOR} = ($version >> 8);
   $self->{VERSION_MINOR} = ($version & 0x0f);
   $self->{GAME} = $client->read_string();
}

#################################################
# Execute a read() packet and collect the data  #
# the client wants                              #
#################################################
sub execute {
   my ( $self, $server, $client, $conf ) = @_;
  
   my $major = $conf->{VERSION_MAJOR};
   my $minor = $conf->{VERSION_MINOR};
   my @server_games = @{$conf->{SERVED_GAMES}};

   $self->{SEND_VERSION} = ( $major << 8 ) + $minor;
   
   if( $major <  $self->{VERSION_MAJOR} ||
       ( $major ==  $self->{VERSION_MAJOR} && $minor < $conf->{VERSION_MINOR})) {
      # Wrong version
      $self->{REP_CODE} = $REP_CODE_PROTOCOL_TO_OLD;
      return;
    }
   
    my $done = 0;
    $self->{GAME} = lc   $self->{GAME};
    
    foreach my $entry (@server_games) {
       $entry = lc $entry;
       if($entry eq  $self->{GAME}) { $done = 1; }
    }
  
    if(! $done ) {
       $self->{REP_CODE} = $REP_CODE_GAME_NOT_SERVED;
       return; 
    }
  
    # Set the clients game
    $client->game($self->{GAME});
    
    # Seems like everything is clear
    $self->{REP_CODE} = $REP_CODE_WELCOME;
};

############################################################
# Send a reply package, this assumes read() and execute    #
# have been called or a error has occured in one of them   #
############################################################
sub write_reply {
   my ( $self, $server, $client, $flags ) = @_;

   $flags = $self->set_is_reply( $flags );

   my $size = 8 + 3 
     ;
   
   $client->write_16( $size );
   $client->write_16( $self->{ID} );
   $client->write_32( $self->{INDEX} );
   $client->write_16( $flags );
   $client->write_8( $self->{REP_CODE} );
   $client->write_16( $self->{SEND_VERSION} );

   if( $self->{REP_CODE} ) {
      # Some error occured. We close this clients connection
      $client->close_connection();
      $server->unregister_client( $client );
   }
   # that's all
};

############################################################# 
#
# Below, set and get functions
#
#############################################################

1;
