import sys
import struct
import zlib

"""
term = ("wings", FileVersion, FileData)
	FileData = (Shapes, Materials, Props)                                                  ## always arity 3
		Shapes = [Object1, Object2, ...]
			ObjectK = ("object", "objectName", objectData, extraData)                      ## always arity 4
				objectData = ("winged", edgeLists, faceMatLists, vertLists, unknownList)   ## always arity 5
					edgeLists = [
						edge1 = [
							("edge",
							  startVert,
							  endVert,
							  leftFace,
							  rightFace,
							  leftPred,
							  leftSucc,
							  rightPred,
							  rightSucc
							),                                                             ## always present
							("color_rt", 12-byte RGB color blob),                          ## right-vertex edge color; optional
							("color_lt", 12-byte RGB color blob),                          ## left-vertex edge color; optional
							("uv_rt", 16-byte blob),                                       ## right-vertex UV coors (8 bytes for U, 8 for V); optional
							("uv_lt", 16-byte blob),                                       ## left-vertex UV coors (8 bytes for U, 8 for V); optional
						], ...
					],
					faceMatLists = [
						faceMat1 = EXT_NIL | [ ("material", "materialName") ],
						faceMat2 = EXT_NIL | [ ("material", "materialName") ], ...
					]
					vertexLists = [
						vertex1 = [ 24-byte blob ],
						vertex2 = [ 24-byte blob ],
						...
					]
		Materials = ?,
		Props = ?,
"""



sunpack = struct.unpack

def ReadInt64(bytes, order, signed):
	assert(len(bytes) == 8)

	if (signed):
		return sunpack('%cq' % order, bytes)[0]
	else:
		return sunpack('%cQ' % order, bytes)[0]

def ReadFloat32(bytes):
	assert(len(bytes) == 4)
	return sunpack('f', bytes)[0]

def ReadInt32(bytes, order, signed):
	assert(len(bytes) == 4)

	if (signed):
		return sunpack('%ci' % order, bytes)[0]
	else:
		return sunpack('%cI' % order, bytes)[0]

def ReadInt16(bytes, order, signed):
	assert(len(bytes) == 2)

	if (signed):
		return sunpack('%ch' % order, bytes)[0]
	else:
		return sunpack('%cH' % order, bytes)[0]



BIG_ENDIAN = '>'
LIT_ENDIAN = '<'

TABS = '\t' * 64
DEBUG = True

ERL_SMALL_INTEGER_EXT =  97  ## uint8 value
ERL_INTEGER_EXT       =  98  ## int32-BE value
ERL_FLOAT_EXT         =  99  ## "%.20e" formatted string, 31 chars
ERL_ATOM_EXT          = 100  ## uint16-BE, variable nr. of 8-bit chars
ERL_SMALL_TUPLE_EXT   = 104  ## uint8 arity
ERL_LARGE_TUPLE_EXT   = 105  ## uint32 (-BE?) arity
ERL_NIL_EXT           = 106  ## empty Erlang list, "[]"
ERL_STRING_EXT        = 107  ## uint16-BE size, characters
ERL_LIST_EXT          = 108  ## uint32? (-BE?) length, head | tail
ERL_BINARY_EXT        = 109  ## uint32-BE length, data (bytes)



def ReadAtom(bytes, readIdx, depth, node):
	size = ReadInt16(bytes[readIdx: readIdx + 2], BIG_ENDIAN, False)
	name = bytes[readIdx + 2: readIdx + 2 + size]

	## should have been done by Wings3D
	## assert(size <= 255)

	if (DEBUG):
		print "%s[ReadAtom] %d \"%s\"" % (TABS[0: depth], size, name)

	return (2 + size)

def ReadString(bytes, readIdx, depth, node):
	size  = ReadInt16(bytes[readIdx: readIdx + 2], BIG_ENDIAN, False)
	chars = bytes[readIdx + 2: readIdx + 2 + size]

	if (DEBUG):
		print "%s[ReadString] \"%s\"" % (TABS[0: depth], chars)

	return (2 + size)

def ReadFloatStr(bytes, readIdx, depth, node):
	s = bytes[readIdx: readIdx + 31]
	i = s.find('\x00')
	s = ((i >= 0 and s[0: i]) or s)
	n = float(s)

	if (DEBUG):
		print "%s[ReadFloatStr] %f" % (TABS[0: depth], n)

	return 31

def ReadSmallInt(bytes, readIdx, depth, node):
	n = ord(bytes[readIdx])

	if (DEBUG):
		print '%s[ReadSmallInt] %d' % (TABS[0: depth], n)

	return 1

def ReadLargeInt(bytes, readIdx, depth, node):
	n = ReadInt32(bytes[readIdx: readIdx + 4], BIG_ENDIAN, True)

	if (DEBUG):
		print '%s[ReadLargeInt] %d' % (TABS[0: depth], n)

	return 4


def ReadTuple(bytes, readIdx, depth, node):
	arity = ord(bytes[readIdx])

	currElIdx  = readIdx + 1
	baseElIdx = currElIdx

	if (DEBUG):
		print "%s[ReadTuple] arity %d" % (TABS[0: depth], arity)

	for i in xrange(arity):
		currElIdx += ReadTerm(bytes, currElIdx, depth + 1, node)

	return (1 + (currElIdx - baseElIdx))

def ReadList(bytes, readIdx, depth, node):
	size = ReadInt32(bytes[readIdx: readIdx + 4], BIG_ENDIAN, False)

	currElIdx  = readIdx + 4
	baseElIdx = currElIdx

	if (DEBUG):
		print "%s[ReadList] %d elements" % (TABS[0: depth], size)

	for i in xrange(size):
		currElIdx += ReadTerm(bytes, currElIdx, depth + 1, node)

	currElIdx += 1
	return (4 + (currElIdx - baseElIdx))

def ReadBinary(bytes, readIdx, depth, node):
	size = ReadInt32(bytes[readIdx: readIdx + 4], BIG_ENDIAN, False)
	## data = bytes[readIdx + 4: readIdx + 4 + size]
	##
	## TODO
	##    if vertex: interpret 24-byte blob as 12-byte pos? + 12-byte normal? (3+3 floats)
	##    if uvcoor: interpret 16-byte blob as 8-byte U-coor + 8-byte V-coor (2 floats)
	##    if color: interpret 12-byte blob as RGB triplet (3 floats)

	if (DEBUG):
		print "%s[ReadBinary] %d bytes" % (TABS[0: depth], size)

	return (4 + size)



def ReadTerm(bytes, readIdx, depth, node):
	termType = ord(bytes[readIdx])

	"""
	if (DEBUG):
		print "%s[ReadTerm] %d %d" % (TABS[0: depth], termType, len(bytes))
	"""

	if (termType == ERL_ATOM_EXT):
		return (1 + ReadAtom(bytes, readIdx + 1, depth + 1, node))

	elif (termType == ERL_SMALL_INTEGER_EXT):
		return (1 + ReadSmallInt(bytes, readIdx + 1, depth + 1, node))

	elif (termType == ERL_INTEGER_EXT):
		return (1 + ReadLargeInt(bytes, readIdx + 1, depth + 1, node))

	elif (termType == ERL_FLOAT_EXT):
		return (1 + ReadFloatStr(bytes, readIdx + 1, depth + 1, node))

	elif (termType == ERL_STRING_EXT):
		return (1 + ReadString(bytes, readIdx + 1, depth + 1, node))

	elif (termType == ERL_SMALL_TUPLE_EXT):
		return (1 + ReadTuple(bytes, readIdx + 1, depth + 1, node))

	elif (termType == ERL_LIST_EXT):
		return (1 + ReadList(bytes, readIdx + 1, depth + 1, node))

	elif (termType == ERL_BINARY_EXT):
		return (1 + ReadBinary(bytes, readIdx + 1, depth + 1, node))

	elif (termType == ERL_NIL_EXT):
		return 1

	else:
		print "%s[ReadTerm] unknown term-type (%d) encountered" % (TABS[0: depth], termType)
		assert(False)
		return 0



def IsWingsFileFormat(header):
	if (    header[0: 11]  != "#!WINGS-1.0"): return False
	if (ord(header[   11]) !=          0x0D): return False
	if (ord(header[   12]) !=          0x0A): return False
	if (ord(header[   13]) !=          0x1A): return False
	if (ord(header[   14]) !=          0x04): return False
	return True

def ReadFile(fname):
	f = open(fname, 'rb')
	s = f.read()
	f = f.close()

	headerBytes = s[0: 15]
	payloadBytes = s[15: ]

	if (not IsWingsFileFormat(headerBytes)):
		return False

	deflatedTermSize = ReadInt32(payloadBytes[0: 4], BIG_ENDIAN, True)
	deflatedTermBytes = payloadBytes[4: ]

	if (ord(deflatedTermBytes[0]) != 131): return False
	if (ord(deflatedTermBytes[1]) !=  80): return False
	if (deflatedTermSize != len(deflatedTermBytes)): return False

	inflatedTermSize = ReadInt32(deflatedTermBytes[2: 6], BIG_ENDIAN, True)
	inflatedTermBytes = zlib.decompress(deflatedTermBytes[6: ])

	bytesRead = ReadTerm(inflatedTermBytes, 0, 0, None)
	return (bytesRead == inflatedTermSize)



if (__name__ == "__main__"):
	for arg in sys.argv[1: ]:
		if (ReadFile(arg)):
			print "successfully read .wings file \"%s\"" % arg
		else:
			print "file \"%s\" is not in .wings format" % arg
