###################################
# Packet base class               #
###################################


package ProtocolPacket_GetUserInfo;

use strict;

use Protocol::ProtocolPacket;
use Carp;


our @ISA = "ProtocolPacket";

# Reply codes

my $UI_ACK = 1;
my $UI_UNKNOWN = 2;


###############################################
# Generate this packet                        #
###############################################
sub init {
   my $self = shift;

   $self->{USER} = undef;
   $self->{ROOM} = undef;
   $self->{GAME} = [ ];
   $self->{REP_CODE} = undef;
}

################################################
# Read this packet, checks for correct syntax  #
################################################
sub read {
   my ( $self, $server, $client ) = @_;
 
   # First read features
   $self->{USER} = $client->read_string();
}

#################################################
# Execute a read() packet and collect the data  #
# the client wants                              #
#################################################
sub execute {
   my ( $self, $server, $client ) = @_;
   
   my $sclient  = $server->get_client_by_name( $self->{USER} );
   
   if( ! $sclient ) {
      $self->{REP_CODE} = $UI_UNKNOWN;
      return;
   }

   $self->{REP_CODE} = $UI_ACK;
   $self->{ROOM} = $sclient->room();
   $self->{GAME} = $sclient->game();
};

############################################################
# Send a reply package, this assumes read() and execute    #
# have been called or a error has occured in one of them   #
############################################################
sub write_reply {
   my ( $self, $server, $client, $flags ) = @_;

   $flags = $self->set_is_reply( $flags );

   my $size = 8 + 1;
   
   if($self->{REP_CODE} != $UI_ACK) {
      $client->write_16( $size );
      $client->write_16( $self->{ID} );
      $client->write_32( $self->{INDEX} );
      $client->write_16( $flags );
      $client->write_8( $self->{REP_CODE} );
      return;
   }
  
   $size += (length($self->{ROOM})+1)*2 
          +(length($self->{GAME})+1)*2;
          

   $client->write_16( $size );
   $client->write_16( $self->{ID} );
   $client->write_32( $self->{INDEX} );
   $client->write_16( $flags );
   $client->write_8( $self->{REP_CODE} );
   $client->write_string ( $self->{GAME} );
   $client->write_string ( $self->{ROOM} );
   
   # that's all
};

1;
