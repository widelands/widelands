###################################
# Server implementation           #
###################################

package Server;
use strict;

# Public uses
use IO::Socket::INET; 
use Carp;

# Our own uses
use Network::ClientHandler;

# Global variables
my $MAX_CONNECTS = 10000;   # How many connections are allowed in maximum
my $SEND_PING_TIME = 10;    # 10 seconds idle time is allowed before ping is send

our ($FIRST_SERVER_PACKET_INDEX, $LAST_SERVER_PACKET_INDEX);

$FIRST_SERVER_PACKET_INDEX = 0xffff0001;
$LAST_SERVER_PACKET_INDEX = 0xffffffff - 1 ;

##########################
# Constructor            #
##########################
sub new {
   my $class = shift;
   my $self = { 
      PORT => 8128,
      IS_BINDED => 0,
      SOCKET => undef,
      CLIENTS =>  [ ], 
      READBUF => "",
      DEBUG => "",
      ROOMS => { }, 
      LAST_INDEX => $FIRST_SERVER_PACKET_INDEX,
      PENDING_PACKETS => { }, 
   };
   if(@_) { $self->{PORT} = shift; } 
   bless($self, $class);
   return $self;
}

###############################
# A new client has identified #
# itself. Add it to the known #
# clients                     #
###############################
sub register_client {
   my $self = shift;
   carp "Network::Server::register_client: takes only the new client" unless(@_);
   my $client = shift;
  
   push @{$self->{CLIENTS}}, $client;
}

###############################
# Remove this client.         #
# He is no longer needed      #
###############################
sub unregister_client {
   my $self = shift;
   my $client = shift;
   carp "Network::Server::unregister_client: takes only the old client" if(@_);

   # Remove him from his ROOM
   $self->client_leaves_room( $client );
   
   for(my $i = 0; $i < scalar (@{$self->{CLIENTS}}) ; $i++ ) {
      if( $self->{CLIENTS}->[$i] == $client) {
         splice @{$self->{CLIENTS}}, $i;
         last;
      }
   }  
}

###############################
# Is this name already in use #
###############################
sub is_username_in_use {
   my $self = shift;
   my $username = shift;

   foreach my $client ( @{$self->{CLIENTS}} ) {
      if($client->username() eq $username) { return 1; }
   }
   return 0;
}

##############################
# Check if there is some new # 
# client pending             #
##############################
sub check_for_connection { 
   my $self = shift;
   unless( $self->{IS_BINDED} ) { $self->_do_bind(); }

   my $sockaddr = accept(my $client_sock, $self->{SOCKET});
   unless ( $sockaddr ) { return undef; }
   # Ok, there came a connection. Create a new client
   ( my $port, my $addy ) = sockaddr_in($sockaddr);
   my $client = Client->new($client_sock, $addy, $port);
   return $client;
}

##########################################
# Is there some data available, has some #
# client written something?              #
##########################################
sub wait_for_clientdata {
   my $self = shift;
   my $time = shift;

   my $rin = '';
   my $rout;
   # now, setup all reads
   foreach my $client ( @{ $self->{CLIENTS}} ) {
      vec($rin, $client->get_socket_number(), 1) = 1;
   }
   
   my $nfound = select($rout=$rin, undef, undef, $time);
  
   if ( $nfound && $self->{DEBUG} > 5 ) {
      print "Found data on $nfound/" . length(@_) . "\n";
   }

   my @need_handling = ();
   foreach my $client (@{ $self->{CLIENTS}} ) {
      push @need_handling, $client unless (vec($rout, $client->get_socket_number(), 1) == 0);
   }
   return @need_handling;
}

###########################################
# A client entered another room,          #
# remove him from the older room          #
# and add him to the new. Also            #
# inform all other clients in this room   #
###########################################
sub client_entered_room {
   my ( $self, $client, $new_room ) = @_;

   # Remove him from his old room, if any
   if($new_room ne $client->room() && $client->room() ne "" ) {
      $self->client_leaves_room( $client );
   }
   
   push @{$self->{ROOMS}->{$new_room}}, $client;
   $client->room($new_room);
   
   foreach my $otclient (@{$self->{ROOMS}->{$new_room}} ) {
      my $flags = 0;
      my $index = $self->get_next_index();
      my $packet = ProtocolPacket->generate_from_name("User_Entered", $index, $flags);

      $packet->construct( $client->username(), $new_room, 1);
      $packet->write( $self , $otclient, $flags);
   
      print "Pushing a pending packet: $index\n";
      $self->{PENDING_PACKETS}->{$index} = $packet;
   }
}

sub client_leaves_room {
   my ( $self, $client ) = @_;

   # Remove this client from the room index
   if( defined (@{$self->{ROOMS}->{$client->room()}}))  {
      for(my $i = 0; $i < scalar ( @{$self->{ROOMS}->{$client->room()}}) ; $i++ ) {
         if( @{$self->{ROOMS}->{$client->room()}}[$i] == $client) {
            splice  @{$self->{ROOMS}->{$client->room()}}, $i, 1;
            last;
         }
      }

      # Send all other clients information that the user has left the room
      foreach my $client (@{$self->{ROOMS}->{$client->room()}} ) {
         my $flags = 0;
         my $index = $self->get_next_index();
         my $packet = ProtocolPacket->generate_from_name("User_Entered", $index, $flags);

         $packet->construct( $client->username(), $client->room(), 0);
         $packet->write( $self , $client, $flags);

         print "Pushing a pending packet: $index\n";
         $self->{PENDING_PACKETS}->{$index} = $packet;
      }
   }
}

#
# Check for non active connections, sending a ping 
# or killing the connection
#
sub check_non_active_connections {
   my $self = shift;

   my $time = time;
   foreach my $client (@{$self->{CLIENTS}}) {
      if( $time-$client->last_activity() > 5*$SEND_PING_TIME) {
         # You're idling. Die!!
         print "Disconnecting this Client!\n";
         $client->disconnet(); 
         $self->unregister_client($client);
      } elsif( $time-$client->last_activity() > $SEND_PING_TIME ) {
         my $flags = 0;
         my $index = $self->get_next_index();
         my $packet = ProtocolPacket->generate_from_name("Ping", $index, $flags);

         $packet->construct();
         $packet->write( $self , $client, $flags);

         print "Sended a Ping Packet!\n";
         $self->{PENDING_PACKETS}->{$index} = $packet;
      }
   }
}

#
# Does this room exist
# 
sub is_room_known {
   my ( $self, $room ) = @_;
   
   return defined (  ( @{$self->{ROOMS}->{$room}}) );
}
   
# 
# Get The rooms users
# 
sub get_room_users {
   my ( $self, $room ) = @_;
   return $self->{ROOMS}->{$room};
}

#
# Get a client by its username
# 
sub get_client_by_name {
   my ( $self, $name ) = @_;

   foreach my $client ( @{$self->{CLIENTS}} ) {
      if( $client->username() eq $name) {
         return $client; 
      }
   }
   return 0;
}

#
# Return a pending packet. It is replied to
#
sub get_pending_packet {
   my ( $self, $index ) = @_;
   
   if( defined ( $self->{PENDING_PACKETS}->{$index} ) ) {
      my $retval = $self->{PENDING_PACKETS}->{$index};
      delete($self->{PENDING_PACKETS}->{$index});
      return $retval ;
   }
   return undef;
}

sub _do_bind {
   my $self = shift; 
   if( $self->{IS_BINDED} ) { confess "Already binded!\n"; }

   socket($self->{SOCKET}, PF_INET, SOCK_STREAM, getprotobyname('tcp'))
      or confess "Couldn't create socket!\n";
   
   # Bind him
   my $sockaddr = sockaddr_in($self->{PORT}, INADDR_ANY);
   bind($self->{SOCKET}, $sockaddr) or confess "bind: $!\n";
   IO::Handle::blocking($self->{SOCKET}, 0); # make handle non blocking
   setsockopt($self->{SOCKET}, SOL_SOCKET, SO_REUSEADDR, 1); # make server restart faster
   
   # make him listen
   listen($self->{SOCKET}, $MAX_CONNECTS); 

   if($self->{DEBUG} ) {
      print "Server: Created socket, binded to " . $self->{PORT} . "\n";
   }
   
   $self->{IS_BINDED} = 1;
}

sub debug {
   my $self = shift;
   if(@_) { $self->{DEBUG} = 1; }
   return $self->{DEBUG};
}

### Returns the next index to be used
sub get_next_index {
   my $self = shift;

   $self->{LAST_INDEX}++;

   if( $self->{LAST_INDEX}>= $LAST_SERVER_PACKET_INDEX ) {
      $self->{LAST_INDEX} = $FIRST_SERVER_PACKET_INDEX; 
   }
   
   return $self->{LAST_INDEX};
}
1;
