import os
import string

#name = ".\Build\Release\GPU_stream_benchmark" #for windows
name = "./GPU_stream_benchmark"	#for linux
#dim = 64
poolDim = 512
#loadMode = 0
#blockMode = 1
testSize = 40
cpuBufferSize = 300

bufferHintList = ["GL_STREAM_DRAW","GL_DYNAMIC_DRAW"]
dimList = [32,33,63,64,65,127,128,129,255,256]
loadModeList = [0, 1, 2]
blockModeList = [1, 2]
textureTypeList = ["Uchar","UcharRGB","UcharRGBA","Float","FloatRGB","FloatRGBA","Int", "IntRGB","IntRGBA"]

for textureType in textureTypeList:
	for blockMode in blockModeList:
		print "blockMode changes"
		for loadMode in loadModeList:
			print "LoadMode changes"
			for dim in dimList:		
				cmd = name+" -dim "+str(dim)+" -poolDim "+str(poolDim)+" -loadMode "+str(loadMode)+" -blockMode " +str(blockMode)+" -testSize "+str(testSize)+" -cpuBuffer "+str(cpuBufferSize)+ " -bufferHint "+bufferHintList[0] + " -textureType " + str(textureType)
				print cmd
				os.system(cmd)

