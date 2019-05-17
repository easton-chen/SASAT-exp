import numpy as np

def file2matrix(filename):
    fr = open(filename)
    arrayLines = fr.readlines()
    lineNumber = len(arrayLines)
    returnMat = np.zeros((lineNumber, 4))
    classLabelVector = []
    index = 0
    for line in arrayLines:
        line = line.strip()
        listFromLine = line.split(',')
        returnMat[index,:] = listFromLine[0:4]
        classLabelVector.append(int(listFromLine[-1]))
        index += 1
    
    return returnMat, classLabelVector

def autoNorm(dataSet):
    minVals = dataSet.min(0)
    maxVals = dataSet.max(0)
    ranges = maxVals - minVals
    normDataSet = np.zeros(np.shape(dataSet))
    m = dataSet.shape[0]
    normDataSet = dataSet - np.tile(minVals, (m,1))
    normDataSet = normDataSet/np.tile(ranges, (m,1))
    return normDataSet

def classify0(inX, dataSet, labels, k):
    dataSetSize = dataSet.shape[0]
    diffMat = np.tile(inX, (dataSetSize,1)) - dataSet
    sqDiffMat = diffMat**2
    sqDistances = sqDiffMat.sum(axis=1)
    distances = sqDistances**0.5
    sortedDistIndicies = distances.argsort()
    classCount = {}
    for i in range(k):
        voteLabel = labels[sortedDistIndicies[i]]
        classCount[voteLabel] = classCount.get(voteLabel,0) + 1
    sortedClassCount = sorted(classCount.items(), key=lambda item:item[1], reverse=True)
    return sortedClassCount[0][0]

def classTest():
    hoRatio = 0.2
    dataMat, dataLabel = file2matrix('env-ss-label.txt')
    normMat = autoNorm(dataMat)
    m = normMat.shape[0]
    numTestVecs = int(m*hoRatio)
    errorCount = 0.0
    for i in range(numTestVecs):
        classifyResult = classify0(normMat[i,:], normMat[numTestVecs:m,:], dataLabel[numTestVecs:m], 7)
        print("predict: %d, expect: %d" % (classifyResult, dataLabel[i]))
        if(classifyResult != dataLabel[i]):
            errorCount += 1.0
    
    print("total error rate is: %f" % (errorCount/float(numTestVecs)))

classTest()


