###################################
# Packet base class               #
###################################


package ProtocolPacket_ChatMessage;

use strict;

use Protocol::ProtocolPacket;
use Carp;


our @ISA = "ProtocolPacket";

# Reply codes

my $CM_ACTION = 1;
my $CM_ACK = 1;


###############################################
# Generate this packet                        #
###############################################
sub init {
   my $self = shift;

   $self->{USER} = undef;
   $self->{MSG} = undef;
   $self->{FLAGS} = undef;
   $self->{REP_CODE} = undef;
}

################################################
# Read this packet, checks for correct syntax  #
################################################
sub read {
   my ( $self, $server, $client ) = @_;
 
   # First read features
   $self->{FLAGS} = $client->read_8();
   $self->{USER} = $client->read_string(); # we ignore this
   $self->{USER} = $client->username();
   $self->{MSG} = $client->read_string();
}

#################################################
# Execute a read() packet and collect the data  #
# the client wants                              #
#################################################
sub execute {
   my ( $self, $server, $client ) = @_;
  
   # Send the data around in the room the user is in
   my @otclients = @{$server->get_room_users( $client->room() )};

   foreach my $otclient (@otclients) {
      my $flags = 0;
      my $index = $server->get_next_index();
      my $packet = ProtocolPacket->generate_from_name("ChatMessage", $index, $flags);

      $packet->construct( $self->{FLAGS}, $self->{USER}, $self->{MSG}) ;
      $packet->write( $server, $otclient, $flags);
  
      $server->{PENDING_PACKETS}->{$index} = $packet;
   }

   $self->{REP_CODE} = $CM_ACK;
};

############################################################
# Send a reply package, this assumes read() and execute    #
# have been called or a error has occured in one of them   #
############################################################
sub write_reply {
   my ( $self, $server, $client, $flags ) = @_;

   $flags = $self->set_is_reply( $flags );

   my $size = 8 + 1;
   
   $client->write_16( $size );
   $client->write_16( $self->{ID} );
   $client->write_32( $self->{INDEX} );
   $client->write_16( $flags );
   $client->write_8( $self->{REP_CODE} );

   # that's all
};

###############################################
# Generate this packet                        #
###############################################
sub construct {
   my $self = shift;
   
   $self->{FLAGS} = shift;
   $self->{USER} = shift; 
   $self->{MSG} = shift; 
}

################################################
# Read this packet, checks for correct syntax  #
################################################
sub write {
   my ( $self, $server, $client, $flags ) = @_;

   my $size = 8 + 1 + 
   (length($self->{USER})+1)*2 + 
   (length($self->{MSG})+1)*2 
   ;

   $client->write_16( $size );
   $client->write_16( $self->{ID} );
   $client->write_32( $self->{INDEX} );
   $client->write_16( $flags );

   $client->write_8 ( $self->{FLAGS} );
   $client->write_string( $self->{USER} );
   $client->write_string( $self->{MSG} );
}

#
# Handle The reply
# 
sub handle_reply {
   my ( $self, $server, $client ) = @_;

   my $ack = $client->read_8();
   
   if($ack != $CM_ACK) {
      # Wrong answer
      print "Client returned illegale answer, was: $ack, should have been $CM_ACK\n";
      return -1;
   }

   return 0;
}


1;
