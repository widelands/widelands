#!/usr/bin/env python3

import struct
import sys

# WARNING!!
# Keep this file in sync with namespace Syncstream in src/logic/game.h

def handle_1(f):
	print("Desync:")
	# Read 4 bytes and interprete them as an signed 32 bit integer
	print("  Desynced player:", struct.unpack('<i', f.read(4))[0])

def handle_2(f):
	print("DestroyObject:")
	print("  Serial:", struct.unpack('<I', f.read(4))[0])

def handle_3(f):
	print("ProcessRequests:")
	print("  Type:", struct.unpack('<B', f.read(1))[0])
	print("  Index:", struct.unpack('<B', f.read(1))[0])
	print("  Serial:", struct.unpack('<I', f.read(4))[0])

def handle_4(f):
	print("HandleActiveSupplies:")
	print("  Size:", struct.unpack('<I', f.read(4))[0])

def handle_5(f):
	print("StartTransfer:")
	print("  Target serial:", struct.unpack('<I', f.read(4))[0])
	print("  Source serial:", struct.unpack('<I', f.read(4))[0])

def handle_6(f):
	print("RunQueue:")
	print("  Duetime:", struct.unpack('<I', f.read(4))[0])
	print("  Command:", struct.unpack('<I', f.read(4))[0])

def handle_7(f):
	print("RandomSeed:")
	print("  Seed:", struct.unpack('<I', f.read(4))[0])

def handle_8(f):
	print("Random:")
	print("  Number:", struct.unpack('<I', f.read(4))[0])

def handle_9(f):
	print("CmdAct:")
	print("  Serial:", struct.unpack('<I', f.read(4))[0])

def handle_A(f):
	print("Battle:")
	print("  Serial first soldier:", struct.unpack('<I', f.read(4))[0])
	print("  Serial second soldier:", struct.unpack('<I', f.read(4))[0])

def handle_B(f):
	print("BobSetPosition:")
	print("  Serial:", struct.unpack('<I', f.read(4))[0])
	print("  Position X:", struct.unpack('<h', f.read(2))[0])
	print("  Position y:", struct.unpack('<h', f.read(2))[0])

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


with open(sys.argv[1], "rb") as f:
	print("Created with %s(%s)"
		% (str(f.read(struct.unpack('<I', f.read(4))[0]).decode()),
		   str(f.read(struct.unpack('<I', f.read(4))[0]).decode())),
		end='')
	playernumber = struct.unpack('<i', f.read(4))[0]
	if playernumber < 0:
		print(" as client")
	else:
		print(" as host, desynced client = %i" % playernumber)
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
			print("Warning: Unknown command code '%s', skipping %i bytes" % (cmd, length))
			for i in range(0, length):
				byte = f.read(1)
		byte = f.read(1)
