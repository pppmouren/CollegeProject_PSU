#Lab #7
#Due Date: 11/12/2021, 11:59PM
# REMINDERS:
#        The work in this assignment must be your own original work and must be completed alone.

class MinBinaryHeap:
    '''
        >>> h = MinBinaryHeap()
        >>> h.insert(10)
        >>> h.insert(5)
        >>> h
        [5, 10]
        >>> h.insert(14)
        >>> h._heap
        [5, 10, 14]
        >>> h.insert(9)
        >>> h
        [5, 9, 14, 10]
        >>> h.insert(2)
        >>> h
        [2, 5, 14, 10, 9]
        >>> h.insert(11)
        >>> h
        [2, 5, 11, 10, 9, 14]
        >>> h.insert(14)
        >>> h
        [2, 5, 11, 10, 9, 14, 14]
        >>> h.insert(20)
        >>> h
        [2, 5, 11, 10, 9, 14, 14, 20]
        >>> h.insert(20)
        >>> h
        [2, 5, 11, 10, 9, 14, 14, 20, 20]
        >>> h.getMin
        2
        >>> h._leftChild(1)
        5
        >>> h._rightChild(1)
        11
        >>> h._parent(1)
        >>> h._parent(6)
        11
        >>> h._leftChild(6)
        >>> h._rightChild(9)
        >>> h.deleteMin()
        2
        >>> h._heap
        [5, 9, 11, 10, 20, 14, 14, 20]
        >>> h.deleteMin()
        5
        >>> h
        [9, 10, 11, 20, 20, 14, 14]
        >>> len(h)
        7
        >>> h.getMin
        9
    '''

    def __init__(self):   # YOU ARE NOT ALLOWED TO MODIFY THE CONSTRUCTOR
        self._heap=[]
        
    def __str__(self):
        return f'{self._heap}'

    __repr__=__str__

    def __len__(self):
        return len(self._heap)

    @property
    def getMin(self):
        # YOUR CODE STARTS HERE
        if len(self._heap) == 0:
            return None
        return self._heap[0]
        
    
    def _parent(self,index):
        # YOUR CODE STARTS HERE
        if index <= 1 or index > len(self._heap):
            return None
        else:
            return self._heap[index//2 - 1]
            
            
    def _leftChild(self,index):
        # YOUR CODE STARTS HERE
        if index < 1 or 2*index > len(self._heap):
            return None
        else:
            return self._heap[2*index - 1]


    def _rightChild(self,index):
        # YOUR CODE STARTS HERE
        if index < 1 or 2*index+1 > len(self._heap):
            return None
        else:
            return self._heap[2*index]


    def insert(self,item):
        # YOUR CODE STARTS HERE
        self._heap.append(item)
        item_index = len(self._heap)
        item_parent = self._parent(item_index)
        while item_parent != None:
            if item_parent > item:
                self._heap[item_index - 1] = item_parent
                self._heap[item_index//2 - 1] = item
                item_index = item_index//2
                item_parent = self._parent(item_index)
            else:
                break
     
        
            

    def deleteMin(self):
        # Remove from an empty heap or a heap of size 1
        if len(self)==0:
            #print('None')
            return None
        elif len(self)==1:
            deleted=self._heap[0]
            self._heap=[]
            #print(deleted)
            return deleted
        else:
            # YOUR CODE STARTS HERE
            a = self._heap.pop(-1)
            b = self._heap.pop(0)
            self._heap.insert(0,a)
            index = 1
            root = self._heap[index - 1]
            left = self._leftChild(index)
            right = self._rightChild(index)
            while left != None:
                if right == None:
                    if root <= left:
                        break
                    else:
                        self._heap[index - 1] = left
                        self._heap[2*index - 1] = root
                        break
                else:
                    if left <= right:
                        if root <= left:
                            break
                        else:
                            self._heap[index - 1] = left
                            self._heap[2*index - 1] = root
                            index = 2*index
                            left = self._leftChild(index)
                            right = self._rightChild(index)
                    else:
                        if root <= right:
                            break
                        else:
                            self._heap[index - 1] = right
                            self._heap[2*index] = root
                            index = 2*index + 1
                            left = self._leftChild(index)
                            right = self._rightChild(index)
        #print(b)
        return b
                        
                
              
                


def heapSort(numList):
    '''
       >>> heapSort([9,1,7,4,1,2,4,8,7,0,-1,0])
       [-1, 0, 0, 1, 1, 2, 4, 4, 7, 7, 8, 9]
       >>> heapSort([-15, 1, 0, -15, -15, 8 , 4, 3.1, 2, 5])
       [-15, -15, -15, 0, 1, 2, 3.1, 4, 5, 8]
    '''
    # YOUR CODE STARTS HERE
    temp = MinBinaryHeap()
    sorted_lst = []
    for i in numList:
        temp.insert(i)
    while len(temp._heap) != 0:
        sorted_lst.append(temp.deleteMin())
    return sorted_lst
        
if __name__ == "__main__":
    #import doctest
    #doctest.testmod()
    h = MinBinaryHeap()
    h._heap = [2, 5, 11, 10, 9, 14, 14, 18, 20]
    h.deleteMin()
    h.deleteMin()
    h.deleteMin()
    h.deleteMin()
    h.deleteMin()
    h.deleteMin()
    h.deleteMin()
    h.deleteMin()
    h.deleteMin()
    h.deleteMin()
    
