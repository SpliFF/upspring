#!BPY

"""
Name: 'Spring OBJ meta-data script (.lua)...'
Blender: 244
Group: 'Export'
Tooltip: 'Generate Spring meta-data script for OBJ models'
"""

__author__  = "Kloot"
__license__ = "GPL v2"
__version__ = "1.0 (March 31, 2010)"

import os
import bpy
import Blender
from Blender import Mesh, Object, Window
import BPyMessages



class SpringModelPiece:
	def __init__(self, sceneObject):
		self.name = sceneObject.getName()
		## instance of type(SpringModelPiece)
		self.parentPiece = None
		## instance of type(BlenderObject)
		self.parentPieceObj = sceneObject.getParent()
		## list of type(SpringModelPiece) instances
		self.childPieces = []
		self.isRoot = (self.parentPieceObj == None)

		## by default, Blender stores an object
		## position in local space if it has a
		## parent
		self.loffsetx = sceneObject.getLocation()[0]
		self.loffsety = sceneObject.getLocation()[1]
		self.loffsetz = sceneObject.getLocation()[2]
		## these are set later
		self.goffsetx = 0.0
		self.goffsety = 0.0
		self.goffsetz = 0.0

	def GetName(self): return self.name
	def IsRoot(self): return self.isRoot

	def SetParentPiece(self, p): self.parentPiece = p
	def GetParentPieceObj(self): return self.parentPieceObj
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

		self.numPieces = 0
		self.rootPiece = None

	def IsEmpty(self):
		return (len(self.pieces.keys()) == 0)
	def HasRootPiece(self):
		return (self.rootPiece != None)

	def AddPiece(self, name, piece):
		self.pieces[name] = piece
	def SetRootPiece(self, modelPiece):
		assert(self.rootPiece == None)
		self.rootPiece = modelPiece

	def SetPieceLinks(self):
		for modelPieceName in self.pieces.keys():
			modelPiece = self.pieces[modelPieceName]

			if (not modelPiece.IsRoot()):
				parentPieceObj = modelPiece.GetParentPieceObj()
				parentPiece = self.pieces[parentPieceObj.getName()]
				parentPiece.AddChildPiece(modelPiece)
				modelPiece.SetParentPiece(parentPiece)

	def CalculateGlobalOffsets(self):
		self.numPieces = len(self.pieces.keys())
		self.rootPiece.CalculateGlobalOffsets(self.midPos)

		## take the geometric average over
		## all global-space piece positions
		self.midPos[0] /= float(self.numPieces)
		self.midPos[1] /= float(self.numPieces)
		self.midPos[2] /= float(self.numPieces)

		minPieceY =  10000
		maxPieceY = -10000
		maxRadius =      0

		for modelPieceName in self.pieces.keys():
			modelPiece = self.pieces[modelPieceName]

			dx = modelPiece.GetGlobalOffsetX() - self.midPos[0]
			dy = modelPiece.GetGlobalOffsetY() - self.midPos[1]
			dz = modelPiece.GetGlobalOffsetZ() - self.midPos[2]

			minPieceY = min(minPieceY, modelPiece.GetGlobalOffsetY())
			maxPieceY = max(maxPieceY, modelPiece.GetGlobalOffsetY())
			maxRadius = max(maxRadius, (((dx * dx) + (dy * dy) + (dz * dz)) ** 0.5))

		## set the radius to the max. distance from any piece to the mid-position
		## set the height to the max. y-difference between two global piece-positions
		## FIXME: be smarter about this?
		self.radius = maxRadius
		self.height = abs(maxPieceY - minPieceY)



def WriteModelPieceTree(modelPiece, depth):
	tabs = ('\t' * (depth + 2))

	s = "%s%s = {\n" % (tabs, modelPiece.GetName())
	s += "%s\toffset = %s,\n" % (tabs, modelPiece.GetLocalOffsetsStr())
	s += "\n"

	for childPiece in modelPiece.GetChildPieces():
		s += WriteModelPieceTree(childPiece, depth + 1)

	s += "},\n"
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
	s += "\ttex1 = \"%s.png\",\n" % (model.name)
	s += "\ttex2 = \"%s.png\",\n" % (model.name)
	s += "\n"
	s += "\tnumpieces = %d, -- includes the root and empty pieces\n" % (model.numPieces)
	s += "\n"
	s += "\tglobalvertexoffsets = %s, -- vertices in global space?\n" % "false"
	s += "\tlocalpieceoffsets = %s, -- offsets in local space?\n" % "true"
	s += "}\n"
	s += "\n"
	s += "return %s\n" % model.name

	return s

def SaveSpringOBJMetaDataScript(filename):
	if (os.path.exists(filename) and (not BPyMessages.Warning_SaveOver(filename))):
		return

	r = False

	scene = Blender.Scene.GetCurrent()
	objects = scene.objects
	model = SpringModel(filename[0: -4])

	## convert the Blender objects to "model pieces"
	for obj in objects:
		if (obj.getType() != "Mesh"):
			continue

		modelPiece = SpringModelPiece(obj)
		model.AddPiece(obj.getName(), modelPiece)

		if (modelPiece.IsRoot()):
			model.SetRootPiece(modelPiece)

	## generate the metadata
	if (not model.IsEmpty()):
		if (model.HasRootPiece()):
			model.SetPieceLinks()
			model.CalculateGlobalOffsets()

			try:
				f = open(filename, "w")
				s = WriteMetaDataString(model)
				f.write(s)
				f = f.close()

				r = True
			except IOError:
				print "[SaveSpringOBJMetaDataScript] ERROR: cannot open \"%s\" for writing" % filename
		else:
			print "[SaveSpringOBJMetaDataScript] ERROR: model does not have a root-piece"
	else:
		print "[SaveSpringOBJMetaDataScript] ERROR: model does not have any pieces"

	Window.WaitCursor(0)
	return r



Window.FileSelector(SaveSpringOBJMetaDataScript, "Spring OBJ meta-data script", "*.lua")
