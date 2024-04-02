# Lab #5
# Due Date: 10/22/2021, 11:59PM
# REMINDERS:
#        The work in this assignment must be your own original work and must be completed alone.


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
        >>> x.isEmpty()
        True
        >>> x.insert(9)
        >>> x.insert(4)
        >>> x.insert(11)
        >>> x.insert(2)
        >>> x.insert(5)
        >>> x.insert(10)
        >>> x.insert(9.5)
        >>> x.insert(7)
        >>> x.getMin
        Node(2)
        >>> x.getMax
        Node(11)
        >>> 67 in x
        False
        >>> 9.5 in x
        True
        >>> x.isEmpty()
        False
        >>> x.getHeight(x.root)   # Height of the tree
        3
        >>> x.getHeight(x.root.left.right)
        1
        >>> x.getHeight(x.root.right)
        2
        >>> x.getHeight(x.root.right.left)
        1
        >>> x.printInorder
        2 : 4 : 5 : 7 : 9 : 9.5 : 10 : 11 :
        >>> new_tree = x.mirror()
        11 : 10 : 9.5 : 9 : 7 : 5 : 4 : 2 :
        >>> new_tree.root.right
        Node(4)
        >>> x.printInorder
        2 : 4 : 5 : 7 : 9 : 9.5 : 10 : 11 : 
    '''
    def __init__(self):
        self.root = None


    def insert(self, value):
        if self.root is None:
            self.root=Node(value)
        else:
            self._insert(self.root, value)


    def _insert(self, node, value):
        if(value<node.value):
            if(node.left==None):
                node.left = Node(value)
            else:
                self._insert(node.left, value)
        else:
            if(node.right==None):
                node.right = Node(value)
            else:
                self._insert(node.right, value)
    
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


    def mirror(self):
        # Creates a new BST that is a mirror of self: Elements greater than the root are on the left side, and smaller values on the right side
        # Do NOT modify any given code
        if self.root is None:
            return None
        else:
            newTree = BinarySearchTree()
            newTree.root = self._mirrorHelper(self.root)
            newTree.printInorder
            return newTree
        




    def isEmpty(self):
        # YOUR CODE STARTS HERE
        return self.root == None
   
    
    
    def _mirrorHelper(self, node):
        # YOUR CODE STARTS HERE
        new_node = Node(node.value)
        if node.right != None:
            new_node.left = self._mirrorHelper(node.right)
            #print(new_node.left)
        if node.left != None:
            new_node.right = self._mirrorHelper(node.left)
            #print(new_node.right)
   
        
        return new_node
        
#if __name__ == "__main__":
#    x=BinarySearchTree()
#    x.insert(9)
#    x.insert(4)
#    x.insert(11)
#    x.insert(2)
#    x.insert(5)
#    x.insert(10)
#    x.insert(9.5)
#    x.insert(7)
#    new_tree = x.mirror()


    @property
    def getMin(self):
        # YOUR CODE STARTS HERE
        if self.isEmpty():
            return None
        else:
            current = self.root
            while current.left != None:
                current = current.left
            return current



    @property
    def getMax(self):
        # YOUR CODE STARTS HERE
        if self.isEmpty():
            return None
        else:
            current = self.root
            while current.right != None:
                current = current.right
            return current



    def __contains__(self, value):
        # YOUR CODE STARTS HERE
        if self.isEmpty():
            return False
        else:
            if self._containsHelper(value,self.root) == True:
                return True
            else:
                return False
    
    def _containsHelper(self, value, node):

        if value == node.value:
            return True
        elif value > node.value:
            if node.right == None:
                return False
            else:
                return self._containsHelper(value, node.right)
        elif value < node.value:
            if node.left == None:
                return False
            else:
                return self._containsHelper(value, node.left)
            
    
    
        
            



    def getHeight(self, node):
        # YOUR CODE STARTS HERE
        a = 0
        if node.right == None and node.left == None:
            return a
        elif node.right != None and node.left != None:
            a = 1
            return a + max(self.getHeight(node.left),self.getHeight(node.right))
        elif node.right != None:
            a = 1
            return a + self.getHeight(node.right)
        elif node.left != None:
            a = 1
            return a + self.getHeight(node.left)

if __name__ == "__main__":
    import doctest
    doctest.testmod()





