#!/usr/bin/env python3

import struct
import sys

# WARNING!!
# Keep this file in sync with the enum SyncEntry in src/logic/game.h


def handle_1(f):
    print('Desync:')
    # Read 4 bytes and interprete them as an signed 32 bit integer
    print('  Desynced player:', struct.unpack('<i', f.read(4))[0])


def handle_2(f):
    print('DestroyObject:')
    print('  Serial:', struct.unpack('<I', f.read(4))[0])


def handle_3(f):
    print('ProcessRequests:')
    print('  Type:', struct.unpack('<B', f.read(1))[0])
    print('  Index:', struct.unpack('<B', f.read(1))[0])
    print('  Serial:', struct.unpack('<I', f.read(4))[0])


def handle_4(f):
    print('HandleActiveSupplies:')
    print('  Size:', struct.unpack('<I', f.read(4))[0])


def handle_5(f):
    print('StartTransfer:')
    print('  Target serial:', struct.unpack('<I', f.read(4))[0])
    print('  Source serial:', struct.unpack('<I', f.read(4))[0])


def handle_6(f):
    print('RunQueue:')
    print('  Duetime:', struct.unpack('<I', f.read(4))[0])
    print('  Command:', struct.unpack('<I', f.read(4))[0])


def handle_7(f):
    print('RandomSeed:')
    print('  Seed:', struct.unpack('<I', f.read(4))[0])


def handle_8(f):
    print('Random:')
    print('  Number:', struct.unpack('<I', f.read(4))[0])


def handle_9(f):
    print('CmdAct:')
    print('  Serial:', struct.unpack('<I', f.read(4))[0])
    # See MapObjectType in map_object.h for decoding the type
    print('  Actor type:', struct.unpack('<B', f.read(1))[0])


def handle_A(f):
    print('Battle:')
    print('  Serial first soldier:', struct.unpack('<I', f.read(4))[0])
    print('  Serial second soldier:', struct.unpack('<I', f.read(4))[0])


def handle_B(f):
    print('BobSetPosition:')
    print('  Serial:', struct.unpack('<I', f.read(4))[0])
    print('  Position X:', struct.unpack('<h', f.read(2))[0])
    print('  Position y:', struct.unpack('<h', f.read(2))[0])


handlers = {
    1: handle_1,
    2: handle_2,
    3: handle_3,
    4: handle_4,
    5: handle_5,
    6: handle_6,
    7: handle_7,
    8: handle_8,
    9: handle_9,
    10: handle_A,
    11: handle_B
}


if len(sys.argv) != 2:
    print('Usage: %s filename.wse > outfile.txt\n' % sys.argv[0])

    print('Parses the hex-data in the given Widelands Syncstream Excerpt')
    print('into human readable form and echos it to stdout.\n')

    print('You should do this with both the *.wse files of the host and')
    print('the desyncing client. Which player desynced is also reported')
    print('at the beginning of the output file of the host.')
    print('Afterwards, the resulting files can be compared with the')
    print("'diff' command. For correctly syncronized games the resulting")
    print('files should be the same, but most likely there will be some')
    print('small differences in the files (e.g., commands that are only')
    print('executed for one of the players).')
    print('There probably will be a large block of commands present at')
    print('the beginning of only one of the files, this is normal and')
    print('can be ignored.\n')

    print('To find out more about the commands and where in the Widelands')
    print('code they are used, see the enum SyncEntry in src/logic/game.h')
    sys.exit(1)

with open(sys.argv[1], 'rb') as f:
    print('Created with %s(%s)'
          % (str(f.read(struct.unpack('<I', f.read(4))[0]).decode()),
             str(f.read(struct.unpack('<I', f.read(4))[0]).decode())),
          end='')
    playernumber = struct.unpack('<i', f.read(4))[0]
    if playernumber < 0:
        print(' as client')
    else:
        print(' as host, desynced client = %i' % playernumber)
    byte = f.read(1)
    while byte:
        # Parse byte
        b = ord(byte)
        cmd = b >> 4
        length = b & 0x0F
        if cmd in handlers:
            handlers[cmd](f)
        else:
            # Unknown command. Skip as many bytes as the entry is long
            print("Warning: Unknown command code '%s', skipping %i bytes" %
                  (cmd, length))
            for i in range(0, length):
                byte = f.read(1)
        byte = f.read(1)
