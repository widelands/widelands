###################################
# Packet base class               #
###################################

# format: 
# id       2
# index    4
# flags    2
# features 1
# username string


package ProtocolPacket_GetRoomInfo;

use strict;

use Protocol::ProtocolPacket;
use Carp;


our @ISA = "ProtocolPacket";

# Reply codes

my $RI_ACK = 1;
my $RI_NONEXISTANT = 2;


###############################################
# Generate this packet                        #
###############################################
sub init {
   my $self = shift;

   $self->{ROOM} = undef;
   $self->{NR_USERS} = undef;
   $self->{USERNAMES} = [ ];
   $self->{REP_CODE} = undef;
}

################################################
# Read this packet, checks for correct syntax  #
################################################
sub read {
   my ( $self, $server, $client ) = @_;
 
   # First read features
   $self->{ROOM} = $client->read_string();
}

#################################################
# Execute a read() packet and collect the data  #
# the client wants                              #
#################################################
sub execute {
   my ( $self, $server, $client ) = @_;
   
   if( ! $server->is_room_known( $self->{ROOM} ) ) {
      $self->{REP_CODE} = $RI_NONEXISTANT;
      return;
   }

   $self->{REP_CODE} = $RI_ACK;
   $self->{USERNAMES} = $server->get_room_users( $self->{ROOM} );
};

############################################################
# Send a reply package, this assumes read() and execute    #
# have been called or a error has occured in one of them   #
############################################################
sub write_reply {
   my ( $self, $server, $client, $flags ) = @_;

   $flags = $self->set_is_reply( $flags );

   my $size = 8 + 1;
   my $nr_users = 0;
   
   if($self->{REP_CODE} != $RI_ACK) {
      $client->write_16( $size );
      $client->write_16( $self->{ID} );
      $client->write_32( $self->{INDEX} );
      $client->write_16( $flags );
      $client->write_8( $self->{REP_CODE} );
      return;
   }
  
   $nr_users = scalar ( @{$self->{USERNAMES}} );
   
   for (my $i=0; $i < $nr_users; $i++) {
      my $name = (@{$self->{USERNAMES}})[$i]->username();
      $size += (length($name)+1)*2;
   } 

   $size+=2 ; # nr of users

   $client->write_16( $size );
   $client->write_16( $self->{ID} );
   $client->write_32( $self->{INDEX} );
   $client->write_16( $flags );
   $client->write_8( $self->{REP_CODE} );
   $client->write_16 ( $nr_users );
  for (my $i=0; $i < $nr_users; $i++) {
      my $name = (@{$self->{USERNAMES}})[$i]->username();
      $client->write_string( $name );
   } 
   # that's all
};

############################################################# 
#
# Below, set and get functions
#
#############################################################
sub username {
   my $self = shift;
   
   if( defined( $self->{USERNAME} )) {
      return $self->{USERNAME};
   }
   $self->{USERNAME} = shift;
}

sub features {
   my $self = shift;
   
   if( defined( $self->{FEATURES} )) {
      return $self->{FEATURES};
   }
   $self->{FEATURES} = shift;
}


1;
