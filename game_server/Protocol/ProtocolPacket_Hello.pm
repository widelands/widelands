###################################
# Packet base class               #
###################################

# format: 
# id       2
# index    4
# flags    2
# features 1
# username string


package ProtocolPacket_Hello;

use strict;

use Protocol::ProtocolPacket;
use Carp;


our @ISA = "ProtocolPacket";

# Reply codes

our $REP_CODE_USER_WELCOME = 1;


###############################################
# Generate this packet                        #
###############################################
sub init {
   my $self = shift;

   $self->{USERNAME} = undef;
   $self->{FEATURES} = undef;
   $self->{REP_CODE} = undef;
   $self->{REP_GROUP} = "users";
   $self->{REP_ROOM} = "lobby";
   $self->{REP_WELCOME} = undef;
}

################################################
# Read this packet, checks for correct syntax  #
################################################
sub read {
   my ( $self, $server, $client ) = @_;
 
   # First read features
   $self->{FEATURES} = $client->read_8();
   $self->{USERNAME} = $client->read_string();
}

#################################################
# Execute a read() packet and collect the data  #
# the client wants                              #
#################################################
sub execute {
   my ( $self, $server, $client ) = @_;
   
   my $username = $self->{USERNAME};
   my $i = 0;
   while( $server->is_username_in_use($username) ) {
      $username = $self->{USERNAME} . $i;
      $i++;
   }
   $self->{USERNAME} = $username;
    
   $client->username ( $username );
  
   # This client is welcome here
   $self->{REP_CODE} = $REP_CODE_USER_WELCOME;
   $self->{REP_WELCOME} = "Welcome " .  $self->{USERNAME} . ".\n" .
      "You are listed in the group " . $self->{REP_GROUP} . "\n" .
      "Have a good time on this Widelands Server!\n";

};

############################################################
# Send a reply package, this assumes read() and execute    #
# have been called or a error has occured in one of them   #
############################################################
sub write_reply {
   my ( $self, $server, $client, $flags ) = @_;

   $flags = $self->set_is_reply( $flags );

   my $size = 8 + 1 + 
     (length($self->{USERNAME})+1)*2 + 
     (length($self->{REP_GROUP})+1)*2 +
     (length($self->{REP_ROOM})+1)*2 +
     (length($self->{REP_WELCOME})+1)*2
     ;
   
   $client->write_16( $size );
   $client->write_16( $self->{ID} );
   $client->write_32( $self->{INDEX} );
   $client->write_16( $flags );
   $client->write_8( $self->{REP_CODE} );
   $client->write_string( $self->{USERNAME} );
   $client->write_string( $self->{REP_GROUP} );
   $client->write_string( $self->{REP_ROOM} );
   $client->write_string( $self->{REP_WELCOME} );

   if( $self->{REP_CODE} == $REP_CODE_USER_WELCOME ) {
      # Inform the server that this client has entered a 
      # room
      $server->client_entered_room($client, $self->{REP_ROOM});
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
