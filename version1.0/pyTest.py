import os
import string

#name = ".\Build\Release\GPU_stream_benchmark" #for windows
name = "./GPU_stream_benchmark"	#for linux
dim = 64
poolDim = 512
loadMode = 0
blockMode = 1
testSize = 30

dimList = [32,33,63,64,65,127,128,129,255,256,257]
loadModeList = [0, 1, 2]
blockModeList = [0, 1, 2]


for blockMode in blockModeList:
	for dim in dimList:
		numBlock = int(700.0/( (dim*dim*dim)/(1024.0*1024.0) ) )
		cmd = name+" -dim "+str(dim)+" -poolDim "+str(poolDim)+" -loadMode "+str(loadMode)+" -blockMode " +str(blockMode)+" -numBlock "+str(numBlock)+" -testSize "+str(testSize)
#print cmd
		os.system(cmd)
