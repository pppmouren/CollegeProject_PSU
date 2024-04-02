# LAB4
# Due Date: 10/08/2021, 11:59PM
# REMINDERS:
#        The work in this assignment must be your own original work and must be completed alone.

class Node:
    def __init__(self, value):
        self.value = value
        self.next = None
    
    def __str__(self):
        return "Node({})".format(self.value)

    __repr__ = __str__
                        
                          
class SortedLinkedList:
    '''
        >>> x=SortedLinkedList()
        >>> x.add(8.76)
        >>> x.add(1)
        >>> x.add(1)
        >>> x.add(1)
        >>> x.add(5)
        >>> x.add(3)
        >>> x.add(-7.5)
        >>> x.add(4)
        >>> x.add(9.78)
        >>> x.add(4)
        >>> x
        Head:Node(-7.5)
        Tail:Node(9.78)
        List:-7.5 -> 1 -> 1 -> 1 -> 3 -> 4 -> 4 -> 5 -> 8.76 -> 9.78
        >>> x.replicate()
        Head:Node(-7.5)
        Tail:Node(9.78)
        List:-7.5 -> -7.5 -> 1 -> 1 -> 1 -> 3 -> 3 -> 3 -> 4 -> 4 -> 4 -> 4 -> 4 -> 4 -> 4 -> 4 -> 5 -> 5 -> 5 -> 5 -> 5 -> 8.76 -> 8.76 -> 9.78 -> 9.78
        >>> x
        Head:Node(-7.5)
        Tail:Node(9.78)
        List:-7.5 -> 1 -> 1 -> 1 -> 3 -> 4 -> 4 -> 5 -> 8.76 -> 9.78
        >>> x.removeDuplicates()
        >>> x
        Head:Node(-7.5)
        Tail:Node(9.78)
        List:-7.5 -> 1 -> 3 -> 4 -> 5 -> 8.76 -> 9.78
        >>> s=SortedLinkedList()
        >>> s.add(4.5)
        >>> s.add(-3)
        >>> s.add(0)
        >>> s.add(5)
        >>> s.add(2)
        >>> s.add(-9)
        >>> s.add(12.7)
        >>> s.add(-3.5)
        >>> s.add(2)
        >>> s.add(4)
        >>> s.add(1)
        >>> s.add(3)
        >>> s
        Head:Node(-9)
        Tail:Node(12.7)
        List:-9 -> -3.5 -> -3 -> 0 -> 1 -> 2 -> 2 -> 3 -> 4 -> 4.5 -> 5 -> 12.7
        >>> s.replicate()
        Head:Node(-9)
        Tail:Node(12.7)
        List:-9 -> -9 -> -3.5 -> -3.5 -> -3 -> -3 -> 0 -> 1 -> 2 -> 2 -> 2 -> 2 -> 3 -> 3 -> 3 -> 4 -> 4 -> 4 -> 4 -> 4.5 -> 4.5 -> 5 -> 5 -> 5 -> 5 -> 5 -> 12.7 -> 12.7
        >>> s.removeDuplicates()
        >>> s
        Head:Node(-9)
        Tail:Node(12.7)
        List:-9 -> -3.5 -> -3 -> 0 -> 1 -> 2 -> 3 -> 4 -> 4.5 -> 5 -> 12.7
        >>> s.add(0)
        >>> s.add(3)
        >>> s
        Head:Node(-9)
        Tail:Node(12.7)
        List:-9 -> -3.5 -> -3 -> 0 -> 0 -> 1 -> 2 -> 3 -> 3 -> 4 -> 4.5 -> 5 -> 12.7
        >>> myList=s.replicate()
        >>> myList
        Head:Node(-9)
        Tail:Node(12.7)
        List:-9 -> -9 -> -3.5 -> -3.5 -> -3 -> -3 -> 0 -> 0 -> 1 -> 2 -> 2 -> 3 -> 3 -> 3 -> 3 -> 3 -> 3 -> 4 -> 4 -> 4 -> 4 -> 4.5 -> 4.5 -> 5 -> 5 -> 5 -> 5 -> 5 -> 12.7 -> 12.7
        >>> myList.removeDuplicates()
        >>> myList
        Head:Node(-9)
        Tail:Node(12.7)
        List:-9 -> -3.5 -> -3 -> 0 -> 1 -> 2 -> 3 -> 4 -> 4.5 -> 5 -> 12.7
        >>> s.add(13)
        >>> s.add(13)
        >>> s
        Head:Node(-9)
        Tail:Node(13)
        List:-9 -> -3.5 -> -3 -> 0 -> 0 -> 1 -> 2 -> 3 -> 3 -> 4 -> 4.5 -> 5 -> 12.7 -> 13 -> 13
        >>> s.removeDuplicates()
        >>> s
        Head:Node(-9)
        Tail:Node(13)
        List:-9 -> -3.5 -> -3 -> 0 -> 1 -> 2 -> 3 -> 4 -> 4.5 -> 5 -> 12.7 -> 13

    '''

    def __init__(self):   # You are not allowed to modify the constructor
        self.head=None
        self.tail=None

    def __str__(self):   # You are not allowed to modify this method
        temp=self.head
        out=[]
        while temp:
            out.append(str(temp.value))
            temp=temp.next
        out=' -> '.join(out)
        return f'Head:{self.head}\nTail:{self.tail}\nList:{out}'

    __repr__=__str__


    def isEmpty(self):
        return self.head == None

    def __len__(self):
        count=0
        current=self.head
        while current:
            current=current.next
            count+=1
        return count

                
    def add(self, value):
        # --- YOUR CODE STARTS HERE
        new_Node = Node(value)
        if self.isEmpty():
            self.head = new_Node
            self.tail = new_Node
        else:
            if new_Node.value >= self.tail.value:
                self.tail.next = new_Node
                self.tail = new_Node
            elif new_Node.value <= self.head.value:
                new_Node.next = self.head
                self.head = new_Node
            else:
                current = self.head
                while current.value < new_Node.value and new_Node.value > current.next.value:
                    current = current.next
                k = current.next
                current.next = new_Node
                new_Node.next = k
                
            
                
                



    def replicate(self):
        # --- YOUR CODE STARTS HERE
        a = SortedLinkedList()
        if self.isEmpty():
            return None
        else:
            current = self.head
            while current is not None:
                if isinstance(current.value, int) and current.value > 0:
                    for i in range(current.value):
                        a.add(current.value)
                elif current.value == 0:
                    a.add(0)
                elif isinstance(current.value, float) or current.value < 0:
                    for i in range(2):
                        a.add(current.value)
                current = current.next
            return a
                
                
            


    def removeDuplicates(self):
        # --- YOUR CODE STARTS HERE
        current = self.head
        while current.next is not None:
            while current.value == current.next.value:
                k = current.next.next
                current.next.next = None
                current.next = k
                if k == None:
                    break
            if current.next == None:
                break
            current = current.next
    
if __name__ == "__main__":
    import doctest
    doctest.testmod()

