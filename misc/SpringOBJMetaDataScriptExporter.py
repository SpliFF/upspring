#!BPY

"""
Name: 'Spring OBJ meta-data script (.lua)...'
Blender: 244
Group: 'Export'
Tooltip: 'Generate Spring meta-data script for OBJ models'
"""

## NOTE: not compatible with Blender 2.5

__author__  = "Kloot"
__license__ = "GPL v2"
__version__ = "1.3 (August 17, 2010)"

import os
import bpy
import Blender
from Blender import Mesh, Object, Window
import BPyMessages



## springrts.com/phpbb/viewtopic.php?t=22635
## www.blender.org/forum/viewtopic.php?t=7214&view=next&sid=b0eadfddb6ef71bf5bfef06dfb75d8a0
## www.blender.org/documentation/249PythonDoc/index.html
## www.blender.org/documentation/250PythonDoc/index.html

## Blender's coordinate-system is XZY, ie. rotated
## 90 degrees clock-wise around the x-axis (facing
## toward the origin) of Spring's XYZ system
##
##   Blender: XYZ ->  Spring: (X)(Z)(-Y)
##    Spring: XYZ -> Blender: (X)(-Z)(Y)
BLENDER_AXIS_X = 0
BLENDER_AXIS_Y = 2
BLENDER_AXIS_Z = 1



class SimpleLog:
	def __init__(self, logName, verbose = True):
		self.verbose = verbose
		self.logFile = open(logName, 'w')
		self.logBuff = ""

	def __del__(self):
		self.logFile.write(self.logBuff)
		self.logFile.close()

	def Flush(self):
		self.logFile.write(self.logBuff)
		self.logBuff = ""

	def Write(self, s):
		if (self.verbose):
			self.logBuff += s



class SpringModelPiece:
	def __init__(self, sceneObject):
		self.name = sceneObject.getName()
		## instance of type(SpringModelPiece)
		self.parentPiece = None
		## instances of type(BlenderObject)
		self.objectPieceBO = sceneObject
		self.parentPieceBO = sceneObject.getParent()
		## list of type(SpringModelPiece) instances
		self.childPieces = []
		self.isRoot = (self.parentPieceBO == None)

		## by default, Blender stores an object
		## position in local space if it has a
		## parent
		## NOTE: this is only true if the object
		## was parented before it was translated,
		## otherwise we cannot get the true local
		## matrix directly
		## self.loffsetx = sceneObject.getLocation()[BLENDER_AXIS_X]
		## self.loffsety = sceneObject.getLocation()[BLENDER_AXIS_Y]
		## self.loffsetz = sceneObject.getLocation()[BLENDER_AXIS_Z]
		self.SetParentOffset()

		## these are set later
		self.goffsetx = 0.0
		self.goffsety = 0.0
		self.goffsetz = 0.0

	def GetName(self): return self.name
	def IsRoot(self): return self.isRoot

	def SetParentOffset(self):
		localMat = None

		if (not self.isRoot):
			parentMat = self.parentPieceBO.getMatrix("worldspace")
			objectMat = self.objectPieceBO.getMatrix("worldspace")

			## L = P^-1 * C
			localMat = objectMat * (parentMat.invert())
		else:
			localMat = self.objectPieceBO.getMatrix("worldspace")

		self.loffsetx =  localMat.translationPart()[BLENDER_AXIS_X]
		self.loffsety = -localMat.translationPart()[BLENDER_AXIS_Y]
		self.loffsetz =  localMat.translationPart()[BLENDER_AXIS_Z]

	def SetParentPiece(self, p): self.parentPiece = p
	def GetparentPieceBO(self): return self.parentPieceBO
	def GetChildPieces(self): return self.childPieces
	def AddChildPiece(self, p): self.childPieces += [p]

	## local offsets are wrt. the parent-piece
	def GetLocalOffsetX(self): return self.loffsetx
	def GetLocalOffsetY(self): return self.loffsety
	def GetLocalOffsetZ(self): return self.loffsetz
	## global offsets are wrt. the root-piece
	def GetGlobalOffsetX(self): return self.goffsetx
	def GetGlobalOffsetY(self): return self.goffsety
	def GetGlobalOffsetZ(self): return self.goffsetz

	def GetLocalOffsetsStr(self): return ("{%.2f, %.2f, %.2f}" % (self.loffsetx, self.loffsety, self.loffsetz))
	def GetGlobalOffsetsStr(self): return ("{%.2f, %.2f, %.2f}" % (self.goffsetx, self.goffsety, self.goffsetz))

	def CalculateGlobalOffsets(self, modelMidPos):
		if (not self.isRoot):
			assert(self.parentPiece != None)
			self.goffsetx = self.loffsetx + self.parentPiece.goffsetx
			self.goffsety = self.loffsety + self.parentPiece.goffsety
			self.goffsetz = self.loffsetz + self.parentPiece.goffsetz

		modelMidPos[0] += self.goffsetx
		modelMidPos[1] += self.goffsety
		modelMidPos[2] += self.goffsetz

		for childPiece in self.childPieces:
			childPiece.CalculateGlobalOffsets(modelMidPos)

class SpringModel:
	def __init__(self, modelName):
		self.name = modelName
		self.radius = 0.0
		self.height = 0.0
		self.midPos = [0.0, 0.0, 0.0]
		self.pieces = {}

		self.rootPiece = None

	def StripName(self):
		## strip the path prefix from the name (if any)
		i = self.name.rfind('/')
		j = self.name.rfind('\\')

		if (i >= 0 and j < 0): self.name = self.name[(i + 1): ]
		if (j >= 0 and i < 0): self.name = self.name[(j + 1): ]

	def GetName(self):
		return self.name

	def IsEmpty(self):
		return (len(self.pieces.keys()) == 0)
	def GetPieceCount(self):
		return len(self.pieces.keys())
	def HasRootPiece(self):
		return (self.rootPiece != None)

	def AddPiece(self, name, piece):
		self.pieces[name] = piece
	def SetRootPiece(self, modelPiece):
		if (self.rootPiece == None):
			self.rootPiece = modelPiece

	def SetPieceLinks(self, log):
		for modelPieceName in self.pieces.keys():
			modelPiece = self.pieces[modelPieceName]

			if (not modelPiece.IsRoot()):
				parentPieceBO = modelPiece.GetparentPieceBO()
				parentPiece = self.pieces[parentPieceBO.getName()]
				parentPiece.AddChildPiece(modelPiece)
				modelPiece.SetParentPiece(parentPiece)

				cn = modelPiece.GetName()
				pn = parentPiece.GetName()
				log.Write("model-piece \"%s\" is a child of \"%s\" (parent has %d children)\n" % (cn, pn, len(parentPiece.GetChildPieces())))

	def CalculateGlobalOffsets(self, log):
		self.rootPiece.CalculateGlobalOffsets(self.midPos)

		## take the geometric average over
		## all global-space piece positions
		self.midPos[0] /= float(self.GetPieceCount())
		self.midPos[1] /= float(self.GetPieceCount())
		self.midPos[2] /= float(self.GetPieceCount())

		log.Write("model mid-position: <%.2f, %.2f, %.2f>\n" % (self.midPos[0], self.midPos[1], self.midPos[2]))

	def SetRadiusAndHeight(self, log):
		minPieceY =  10000
		maxPieceY = -10000
		maxRadius =      0

		for modelPieceName in self.pieces.keys():
			modelPiece = self.pieces[modelPieceName]

			dx = modelPiece.GetGlobalOffsetX() - self.midPos[0]
			dy = modelPiece.GetGlobalOffsetY() - self.midPos[1]
			dz = modelPiece.GetGlobalOffsetZ() - self.midPos[2]
			rr = ((dx * dx) + (dy * dy) + (dz * dz))

			minPieceY = min(minPieceY, modelPiece.GetGlobalOffsetY())
			maxPieceY = max(maxPieceY, modelPiece.GetGlobalOffsetY())
			maxRadius = max(maxRadius, (rr ** 0.5))

			log.Write("mid-pos distance of piece \"%s\": %.2f, {min, max}Y: %.2f, %.2f\n" % (modelPieceName, (rr ** 0.5), minPieceY, maxPieceY))

		## set the radius to the max. distance from any piece to the mid-position
		## set the height to the max. y-difference between two global piece-positions
		## FIXME: be smarter about this, ie. read vertices to determine radius and height?
		self.radius = maxRadius
		self.height = abs(maxPieceY - minPieceY)



def WriteModelPieceTree(modelPiece, depth):
	tabs = ('\t' * (depth + 2))

	s = "%s%s = {\n" % (tabs, modelPiece.GetName())
	s += "%s\toffset = %s,\n" % (tabs, modelPiece.GetLocalOffsetsStr())
	s += "\n"

	for childPiece in modelPiece.GetChildPieces():
		s += WriteModelPieceTree(childPiece, depth + 1)

	s += "%s},\n" % (tabs)
	return s

def WriteMetaDataString(model):
	s = "-- Spring metadata for %s.obj\n" % (model.name)
	s += "%s = {\n" % model.name
	s += "\tpieces = {\n"
	s += WriteModelPieceTree(model.rootPiece, 0)
	s += "\t},\n"
	s += "\n"
	s += "\tradius = %.2f,\n" % (model.radius)
	s += "\theight = %.2f,\n" % (model.height)
	s += "\tmidpos = {%.2f, %.2f, %.2f},\n" % (model.midPos[0], model.midPos[1], model.midPos[2])
	s += "\n"
	s += "\ttex1 = \"%s1.png\",\n" % (model.name)
	s += "\ttex2 = \"%s2.png\",\n" % (model.name)
	s += "\n"
	s += "\tnumpieces = %d, -- includes the root and empty pieces\n" % (model.GetPieceCount())
	s += "\n"
	s += "\tglobalvertexoffsets = %s, -- vertices in global space?\n" % "false"
	s += "\tlocalpieceoffsets = %s, -- offsets in local space?\n" % "true"
	s += "}\n"
	s += "\n"
	s += "return %s\n" % (model.name)

	return s

def SaveSpringOBJMetaDataScript(filename):
	if (os.path.exists(filename) and (not BPyMessages.Warning_SaveOver(filename))):
		return False

	r = False

	## FIXME: export only selected objects?
	scene = Blender.Scene.GetCurrent()
	objects = scene.objects
	model = SpringModel(filename[0: -4])
	model.StripName()

	log = SimpleLog(filename[0: -4] + ".log")
	log.Write("[SpringOBJMetaDataScriptExporter] export log for model \"%s\"\n\n" % (model.GetName()))

	## convert the Blender objects to "model pieces"
	for obj in objects:
		objType = obj.getType()

		if ((objType != "Mesh") and (objType != "Empty")):
			log.Write("skipping non-mesh object \"%s\" (type \"%s\")\n" % (obj.getName(), objType))
			continue
		elif (objType == "Mesh"):
			## triangulate selected faces
			mesh = obj.getData(mesh = True)
			mesh.quadToTriangle()

		modelPiece = SpringModelPiece(obj)
		model.AddPiece(obj.getName(), modelPiece)

		if (modelPiece.IsRoot()):
			model.SetRootPiece(modelPiece)

		offsetsStr = modelPiece.GetLocalOffsetsStr()
		isRootStr = ((modelPiece.IsRoot() and "false") or "true")

		log.Write("processed object \"%s\" with offsets %s (has parent: %s)\n" % (obj.getName(), offsetsStr, isRootStr))

	log.Write("model has %d pieces (has root-piece: %s)\n" % (model.GetPieceCount(), ((model.HasRootPiece() and "true") or "false")))

	## generate the metadata
	if (not model.IsEmpty()):
		if (model.HasRootPiece()):
			model.SetPieceLinks(log)
			model.CalculateGlobalOffsets(log)
			model.SetRadiusAndHeight(log)

			try:
				f = open(filename, "w")
				s = WriteMetaDataString(model)
				f.write(s)
				f = f.close()

				r = True
			except IOError:
				log.Write("ERROR: cannot open file \"%s\" for writing\n" % (filename))
		else:
			log.Write("ERROR: model does not have a root-piece\n")
	else:
		log.Write("ERROR: model does not have any pieces\n")

	log.Flush()
	Window.WaitCursor(0)

	del log
	return r



Window.FileSelector(SaveSpringOBJMetaDataScript, "Export to Spring OBJ meta-data script", "*.lua")
