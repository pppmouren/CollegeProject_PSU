def isMinHeap(aTree):
    for i in range(1,(len(aTree)//2) +1):
        if aTree[i-1] > aTree[2*i-1] or (2*i!=len(aTree) and aTree[i-1] > aTree[2*i])
            return False
    return True
