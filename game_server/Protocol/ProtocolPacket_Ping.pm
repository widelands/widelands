###################################
# Packet base class               #
###################################

# format: 
# id       2
# index    4
# flags    2
# features 1
# username string


package ProtocolPacket_UserEntered;

use strict;

use Protocol::ProtocolPacket;
use Carp;


our @ISA = "ProtocolPacket";

# Reply codes

our $UEP_ACK = 1;


###############################################
# Generate this packet                        #
###############################################
sub construct {
   my $self = shift;
   
   $self->{NAME} = shift;
   $self->{ROOM} = shift; 
   $self->{ENTERS} = shift; 
}

################################################
# Read this packet, checks for correct syntax  #
################################################
sub write {
   my ( $self, $server, $client, $flags ) = @_;

   my $size = 8 + 1 + 
   (length($self->{NAME})+1)*2 + 
   (length($self->{ROOM})+1)*2 
   ;

   $client->write_16( $size );
   $client->write_16( $self->{ID} );
   $client->write_32( $self->{INDEX} );
   $client->write_16( $flags );

   $client->write_string( $self->{NAME} );
   $client->write_string( $self->{ROOM} );
   $client->write_8 ( $self->{ENTERS} );
}

#
# Handle The reply
# 
sub handle_reply {
   my ( $self, $server, $client ) = @_;

   my $ack = $client->read_8();
   
   if($ack != $UEP_ACK) {
      # Wrong answer
      print "Client returned illegale answer, was: $ack, should have been $UEP_ACK\n";
      return -1;
   }

   return 0;
}
1;
