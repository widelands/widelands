###################################
# Packet base class               #
###################################

package ProtocolPacket;
use strict;
use Carp;

our (%reference_hash);

# all our protocol packages
use Protocol::ProtocolPacket_Connect;
use Protocol::ProtocolPacket_Hello;
use Protocol::ProtocolPacket_UserEntered;
use Protocol::ProtocolPacket_GetRoomInfo;
use Protocol::ProtocolPacket_GetUserInfo;
use Protocol::ProtocolPacket_ChatMessage;
use Protocol::ProtocolPacket_Ping;


%reference_hash= (
   "1" => { name => "Connect", class=> "ProtocolPacket_Connect" },
   "2" => { name => "Hello", class=> "ProtocolPacket_Hello" },
   "3" => { name => "User_Entered", class=> "ProtocolPacket_UserEntered" },
   "4" => { name => "GetRoomInfo", class=> "ProtocolPacket_GetRoomInfo" },
   "5" => { name => "GetUserInfo", class=> "ProtocolPacket_GetUserInfo" },
   "6" => { name => "ChatMessage", class=> "ProtocolPacket_ChatMessage" },
   "7" => { name => "Ping", class=> "ProtocolPacket_Ping" },
);

############################################
# This is a base class for the protocolls  #
# It also knows how to create the various  # 
# packets.                                 #
# This class fails on most functions,      #
# so you could consider this an abstract   #
# base class                               #
# 
# For the documentation of the 
#  General Game Server Protocoll see the
#  widelands source code (game_server_proto.h)
############################################

################################################
# Constructor, expects an integer identifiying #
# the correct package to use                   #
################################################
sub generate_from_id {
   my $class = shift;
   carp "ProtocolPacket::generate_from_id: needs id as argument!\n" unless @_;
   my $id = shift;
   my $index = shift;
   
   if( defined( $reference_hash{$id} ) ) {
      # Create a class of this. 
      my $gen_class = { };
      
      bless $gen_class, $reference_hash{$id}->{class};

      # init this class
      $gen_class->init();
     
      $gen_class->{ID} = $id;
      $gen_class->{INDEX} = $index;

      return $gen_class; 
   } else {
      confess "Unknown package with id $id, TODO: close this client's connection!\n";
   }
}

sub generate_from_name {
   my ( $self, $name );
   $self = shift;
   $name = shift;

   foreach my $key (keys %reference_hash) {
      if ($reference_hash{$key}->{name} eq $name) {
         return $self->generate_from_id($key, @_);
      }
   }
}

################ Basic init
sub init {
}

################################################
# Read this packet, checks for correct syntax  #
################################################
sub read {
   confess "Generic ProtocolPacket cannot read!\n";
}

################################################
# Write this packet                            #
################################################
sub write {
   confess "Generic ProtocolPacket cannot write!\n";
}

#################################################
# Execute a read() packet and collect the data  #
# the client wants                              #
#################################################
sub execute {
   confess "Generic ProtocolPacket cannot execute!\n";
};

############################################################
# Send a reply package, this assumes read() and execute    #
# have been called or a error has occured in one of them   #
############################################################
sub write_reply {
   confess "Generic ProtocolPacket cannot send a reply\n";
};

#
# A reply must be read
#
sub handle_reply {
   confess "Generic ProtocolPacket cannot send a reply\n";
}

#############################################################
# Return the name of this packet from this id. This can be 
# called either for a class or generic
#############################################################
sub get_name_by_id {
   my $whatever = shift;
   carp "ProtocolPacket::get_name_by_id needs id as argument!\n" unless @_;
   my $id = shift;

   return $reference_hash{$id}->{name};
}

##########################
#                        #
# Set a flag             #
#                        #
##########################
sub set_is_reply {
   my ( $whatever, $flags ) = @_;
   $flags |= 1;
   return $flags;
}

1;
