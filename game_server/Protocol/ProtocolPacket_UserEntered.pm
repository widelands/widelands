###################################
# Packet base class               #
###################################

package ProtocolPacket_Ping;

use strict;

use Protocol::ProtocolPacket;
use Carp;


our @ISA = "ProtocolPacket";

# Reply codes

our $PING_ACK = 1;


###############################################
# Generate this packet                        #
###############################################
sub construct {
   my $self = shift;
}   

################################################
# Read this packet, checks for correct syntax  #
################################################
sub write {
   my ( $self, $server, $client, $flags ) = @_;

   my $size = 8 ;

   $client->write_16( $size );
   $client->write_16( $self->{ID} );
   $client->write_32( $self->{INDEX} );
   $client->write_16( $flags );
}

#
# Handle The reply
# 
sub handle_reply {
   my ( $self, $server, $client ) = @_;

   my $ack = $client->read_8();
   
   if($ack != $PING_ACK) {
      # Wrong answer
      print "Client returned illegale answer, was: $ack, should have been $PING_ACK\n";
      return -1;
   }

   return 0;
}
1;
