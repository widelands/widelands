
package Client;
use strict;

use Carp;
use Socket;

# Global variables
my $DEBUG = 0;

sub new {
   my $class = shift;
   my $self = { 
      SOCKET => shift,
      ADDR => shift,
      PORT => shift,
      USERNAME => "",
      READBUF => "",
      ROOM => "", 
   };
   bless ($self, $class);
   $self->{USERNAME} = $self->{ADDR} . ":" . $self->{PORT};
   IO::Handle::blocking($self->{SOCKET}, 0); # make handle non blocking
   return $self;
}

# Turn debugging on or off
sub debug {
   my $self = shift;
   confess "Debug takes one argument" unless @_ == 1;

   $Client::DEBUG = shift;
}

# Returns a name for this client or sets it
sub username {
   my $self = shift;
   if(@_) { $self->{USERNAME} = shift; }
   return $self->{USERNAME}; 
}

sub room {
   my $self = shift;
   if(@_) { $self->{ROOM} = shift; }
   return $self->{ROOM}; 
}

sub game {
   my $self = shift;
   if(@_) { $self->{GAME} = shift; }
   return $self->{GAME}; 
}

sub last_activity {
   my $self = shift;
   if(@_) { $self->{LAST_ACTIVITY} = shift; }
   return $self->{LAST_ACTIVITY}; 
}

#######################################################
# Return the file descr number of our socket          #
#######################################################
sub get_socket_number {
   my $self = shift;

   return fileno($self->{SOCKET});
}

# 
# Is this client still connected?
# 
sub is_connected {
   my $self = shift;
  
   if(! $self->{SOCKET} ) { return 0; }
   
   $self->_read_all();
   if($self->{READBUF}) { return 1; }
   return 0;
}

#
# Is data available from this client
#
sub has_data {
   my $self = shift;

   if( !$self->is_connected() ) {
      return 0;
   }

   $self->_read_all();
   if($self->{READBUF}) { return 1; }
   return 0;
}

#
# close this connection
#
sub close_connection {
   my $self = shift;
   
   $self->_read_all();
   close $self->{SOCKET} ;
   $self->{READBUF} = 0;
   $self->{SOCKET} = 0;
}

#######################################################
# Read functions                                      #
#######################################################
sub read_line {
   my $self = shift;
   $self->_read_all();

   my $line = 0;
   if($self->{READBUF} =~ s/(^.*?)\r*\n//) {
      # Complete line found
      $line = $1;
   }
   return $line;
}

sub read_16 {
   my $self = shift;
   $self->_read_all();
   
   my ( $short, $newbuf ) = unpack("na*", $self->{READBUF});
   $self->{READBUF} = $newbuf;
   return $short;
};

sub read_32 {
   my $self = shift;
   $self->_read_all();
   
   my ( $short, $newbuf ) = unpack("Na*", $self->{READBUF});
   $self->{READBUF} = $newbuf;
   return $short;
};

sub read_8 {
   my $self = shift;
   $self->_read_all();

   my( $long, $newbuf ) = unpack("Ca*", $self->{READBUF});
   $self->{READBUF} = $newbuf;
   return $long;
}

sub read_string {
   my $self = shift;
   $self->_read_all();

   my ( $i, $len, $buf );
   
   $len = $self->read_16(); # read the number of characters

   for($i = 0; $i < $len; $i++) {
      my $char = $self->read_16();
      $buf .= chr($char); 
   }
   return $buf;
}

#######################################################
# Private Read Functions                              #
#######################################################
sub _read_all {
   my $self = shift;

   my $nread = sysread ($self->{SOCKET}, my $input, 1000);   # 1000 bytes at once ought to be enough

   if($nread && $Client::DEBUG > 5) {
      print "Read " . $nread . " bytes from socket!\n"; 
   }

   $self->{READBUF} .= $input; 
};

#######################################################
# Write functions                                     #
#######################################################
sub write_line {
   my $self = shift;
   
   my $line = shift;
   
   syswrite $self->{SOCKET}, $line;
};

sub write_16 {
   my $self = shift;
   syswrite( $self->{SOCKET}, pack("n", shift), 2);
};

sub write_32 {
   my $self = shift;
   syswrite( $self->{SOCKET}, pack("N", shift), 4);
};

sub write_8 {
   my $self = shift;
   syswrite( $self->{SOCKET}, pack("C", shift), 1);
}

sub write_string {
   my $self = shift;
   my ( $i, $len, $buf );
   
   $buf = shift;
   
   $len = length($buf); 

   $self->write_16($len);
   for($i = 0; $i < $len; $i++) {
      my $char = ord( substr( $buf, $i, 1) );
   $self->write_16($char);
   }
   return $buf;
}

1;
