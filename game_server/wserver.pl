#!/usr/bin/perl -w  

###############################################################
# This programm is the widelands game server. 
#
# This is a platform for gamers to meet and chat, start games
# and maybe more in future. The game<->server communication consits
# of very few packets used for communications. 
#
# The user itself will be able to send server side (IRC like) commands starting
# with / (e.g. /msg, /me). These commands do not need to be representated
# in the game (the client needs no implementation), the response
# is simply server side; or, if it changes something the client needs
# to know, it triggers a normal packet to inform the client
###############################################################


# Options
use strict;

# Includes
use Network::Server;
use Network::ClientHandler;
use Protocol::ProtocolPacket;

my $IS_ANSWER = 1;

# Defines 
my $CONF = {
   VERSION_MAJOR => 1,
   VERSION_MINOR => 0,
   SERVED_GAMES => [ "Chat", "Widelands" ]
};


# Program starts here
my $server = Server->new();
$server->debug(1);
Client->debug(15);

for(;;) {
   my $client = $server->check_for_connection();

   if( $client ) {
      $server->register_client( $client );
   }

   # now check all clients, if there is something to do
   # We wait 5 seconds, this reduces load 
   my @clients_to_handle = $server->wait_for_clientdata(0.25);
   
   foreach $client (@clients_to_handle) {
      unless( $client->is_connected() ) {
         print "Client is disconnected!\n";
         $server->unregister_client( $client );
         next;
      }

      while( $client->has_data() ) {
         $client->last_activity( time );

         my $size = $client->read_16(); 
         my $id = $client->read_16();
         my $index = $client->read_32();
         my $flags = $client->read_16();

         if($flags && $IS_ANSWER) {
            my $packet = $server->get_pending_packet($index);
            print "Got an answer packet with index $index\n";
            if( ! defined ( $packet ) || $id != $packet->{ID} ) {
               # The packet is not known. We disconnet this user
               print "Some error with this packet: sended id: $id, packet id: $packet->{ID}\n";
               $client->close_connection();
               $server->unregister_client($client);
               next;
            }
           
            if($packet->handle_reply( $server, $client ) < 0 ) {
               $client->close_connection();
               $server->unregister_client($client);
            }
         } else {
            my $packet = ProtocolPacket->generate_from_id($id,$index); 
            $packet->read($server, $client);

            print "Got a packet <" . $packet->get_name_by_id($id) . ">\n";

            $packet->execute( $server, $client, $CONF );
            $packet->write_reply( $server, $client, 0 );
         }
      }
   }

   # Check for non active connections
   $server->check_non_active_connections();
}
