# HW4
# Due Date: 11/05/2021, 11:59PM
# REMINDER:
#       The work in this assignment must be your own original work and must be completed alone.
#       You might add additional methods to encapsulate and simplify the operations, but they must be
#       thoroughly documented


class Node:
    def __init__(self, value):
        self.value = value
        self.left = None
        self.right = None
        
    def __str__(self):
        return ("Node({})".format(self.value))

    __repr__ = __str__


class BinarySearchTree:
    '''
        >>> x=BinarySearchTree()
        >>> x.insert('mom')
        >>> x.insert('omm')
        >>> x.insert('mmo')
        >>> x.root
        Node({'mmo': ['mom', 'omm', 'mmo']})
        >>> x.insert('sat')
        >>> x.insert('kind')
        >>> x.insert('ats')
        >>> x.root.left
        Node({'ast': ['sat', 'ats']})
        >>> x.root.right is None
        True
        >>> x.root.left.right
        Node({'dikn': ['kind']})
    '''

    def __init__(self):
        self.root = None


    # Modify the insert and _insert methods to allow the operations given in the PDF
    def insert(self, value):
        if self.root is None:
            data = {}
            temp = ''.join(sorted(value))
            data[temp] = [value]
            self.root=Node(data)
        else:
            self._insert(self.root, value)
        #print(self.root)
        #print(self.root.left)

    def _insert(self, node, value):
        temp = ''.join(sorted(value))
        lst = list(node.value.keys())
        lst.append(temp)
        lst.sort()
        #print(lst)
        if temp in list(node.value.keys()):
            if value not in node.value[temp]:
                temp_lst = node.value[temp]
                temp_lst.append(value)
                node.value[temp] = temp_lst
       
        elif temp == lst[0]:
            if(node.left==None):
                data = {}
                data[temp] = [value]
                node.left = Node(data)
            else:
                self._insert(node.left, value)
        elif temp == lst[1]:
            if(node.right==None):
                data = {}
                data[temp] = [value]
                node.right = Node(data)
                #print(node,node.right)
            else:
                self._insert(node.right, value)


    def isEmpty(self):
        return self.root == None

    @property
    def printInorder(self):
        if self.isEmpty():
            return None
        else:
            self._inorderHelper(self.root)
        
    def _inorderHelper(self, node):
        if node is not None:
            self._inorderHelper(node.left)
            print(node.value, end=' : ')
            self._inorderHelper(node.right)





class Anagrams:
    '''
        # Verify class has _bst attribute
        >>> x = Anagrams(5)
        >>> '_bst' in x.__dict__
        True
        >>> isinstance(x.__dict__.get('_bst'), BinarySearchTree)
        True
        >>> x = Anagrams(5)
        >>> x.create('words_small.txt')
        >>> x.getAnagrams('tap')
        'No match'
        >>> x.getAnagrams('arm')
        'No match'
        >>> x.getAnagrams('rat')
        ['art', 'tar', 'rat']
        >>> x.getAnagrams('sand')
        ['ands', 'sand']
        >>> x._bst.printInorder
        {'a': ['a']} : {'adns': ['ands', 'sand']} : {'ahms': ['sham', 'hams']} : {'amt': ['tam', 'mat']} : {'arst': ['arts', 'rats', 'star']} : {'arsty': ['artsy']} : {'art': ['art', 'tar', 'rat']} : 
        >>> x = Anagrams(5)
        >>> x.create('words_medium.txt')
        >>> x.getAnagrams('sale')
        ['ales', 'leas', 'sale', 'seal']
        >>> x.getAnagrams('love')
        'No match'
        >>> x.getAnagrams('mean')
        ['amen', 'mane', 'mean', 'name']
        >>> x = Anagrams(5)
        >>> x.create('words_large.txt')
        >>> x.getAnagrams('mart')
        ['mart', 'tram']
        >>> x.getAnagrams('each')
        ['each', 'ache']
        >>> x.getAnagrams('oval')
        ['oval']
        >>> x.getAnagrams('rat')
        ['rat', 'tar', 'art']
        '''
    
    def __init__(self, word_size):
        # -YOUR CODE STARTS HERE
        self.word_size = word_size
        self._bst = BinarySearchTree()




    def create(self, file_name):
        # -YOUR CODE STARTS HERE
        # Code for reading the contents of file_name is given in the PDF
        with open(file_name) as f:
            contents = f.read()
        lst1 = contents.splitlines()
        lst1 = ' '.join(lst1)
        lst1 = lst1.split()
        lst2 = []
        for i in lst1:
            if len(i) <= self.word_size:
                lst2.append(i)
        for i in lst2:
            self._bst.insert(i)
            
            
            
    def getAnagrams(self, word):
        # -YOUR CODE STARTS HERE
        temp = sorted(word)
        temp = ''.join(temp)
        current = self._bst.root
        while current is not None:
            lst = list(current.value.keys())
            lst.append(temp)
            lst.sort()
            if temp in list(current.value.keys()):
                return current.value[temp]
            else:
                if temp == lst[0]:
                    current = current.left
                elif temp == lst[1]:
                    current = current.right
        return 'No match'
                
                
            
        
        
        
    
        
        
if __name__ == "__main__":
    import doctest
    doctest.testmod()

