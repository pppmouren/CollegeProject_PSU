# HW5
# Due Date: 11/19/2021, 11:59PM
# REMINDER:
#       The work in this assignment must be your own original work and must be completed alone.


class Node:
    def __init__(self, content):
        self.value = content
        self.next = None

    def __str__(self):
        return ('CONTENT:{}\n'.format(self.value))

    __repr__=__str__


class ContentItem:
    '''
        >>> content1 = ContentItem(1000, 10, "Content-Type: 0", "0xA")
        >>> content2 = ContentItem(1004, 50, "Content-Type: 1", "110010")
        >>> content3 = ContentItem(1005, 18, "Content-Type: 2", "<html><p>'CMPSC132'</p></html>")
        >>> content4 = ContentItem(1005, 18, "another header", "111110")
        >>> hash(content1)
        0
        >>> hash(content2)
        1
        >>> hash(content3)
        2
        >>> hash(content4)
        1
    '''
    def __init__(self, cid, size, header, content):
        self.cid = cid
        self.size = size
        self.header = header
        self.content = content

    def __str__(self):
        return f'CONTENT ID: {self.cid} SIZE: {self.size} HEADER: {self.header} CONTENT: {self.content}'

    __repr__=__str__

    def __eq__(self, other):
        if isinstance(other, ContentItem):
            return self.cid == other.cid and self.size == other.size and self.header == other.header and self.content == other.content
        return False

    def __hash__(self):
        # YOUR CODE STARTS HERE
        hash_value = 0
        for i in self.header:
            hash_value += ord(i)
        pos = hash_value % 3
        return pos

    
    
class CacheList:
    '''
        # An extended version available on Canvas. Make sure you pass this doctest first before running the extended version

        >>> content1 = ContentItem(1000, 10, "Content-Type: 0", "0xA")
        >>> content2 = ContentItem(1004, 50, "Content-Type: 1", "110010")
        >>> content3 = ContentItem(1005, 180, "Content-Type: 2", "<html><p>'CMPSC132'</p></html>")
        >>> content4 = ContentItem(1006, 18, "another header", "111110")
        >>> content5 = ContentItem(1008, 2, "items", "11x1110")
        >>> lst=CacheList(200)
        >>> lst
        REMAINING SPACE:200
        ITEMS:0
        LIST:
        <BLANKLINE>
        >>> lst.put(content1, 'mru')
        'INSERTED: CONTENT ID: 1000 SIZE: 10 HEADER: Content-Type: 0 CONTENT: 0xA'
        >>> lst.put(content2, 'lru')
        'INSERTED: CONTENT ID: 1004 SIZE: 50 HEADER: Content-Type: 1 CONTENT: 110010'
        >>> lst.put(content4, 'mru')
        'INSERTED: CONTENT ID: 1006 SIZE: 18 HEADER: another header CONTENT: 111110'
        >>> lst.put(content5, 'mru')
        'INSERTED: CONTENT ID: 1008 SIZE: 2 HEADER: items CONTENT: 11x1110'
        >>> lst.put(content3, 'lru')
        "INSERTED: CONTENT ID: 1005 SIZE: 180 HEADER: Content-Type: 2 CONTENT: <html><p>'CMPSC132'</p></html>"
        >>> lst.put(content1, 'mru')
        'INSERTED: CONTENT ID: 1000 SIZE: 10 HEADER: Content-Type: 0 CONTENT: 0xA'
        >>> 1006 in lst
        True
        >>> contentExtra = ContentItem(1034, 2, "items", "other content")
        >>> lst.update(1008, contentExtra)
        'UPDATED: CONTENT ID: 1034 SIZE: 2 HEADER: items CONTENT: other content'
        >>> lst
        REMAINING SPACE:170
        ITEMS:3
        LIST:
        [CONTENT ID: 1034 SIZE: 2 HEADER: items CONTENT: other content]
        [CONTENT ID: 1006 SIZE: 18 HEADER: another header CONTENT: 111110]
        [CONTENT ID: 1000 SIZE: 10 HEADER: Content-Type: 0 CONTENT: 0xA]
        <BLANKLINE>
        >>> lst.clear()
        'Cleared cache!'
        >>> lst
        REMAINING SPACE:200
        ITEMS:0
        LIST:
        <BLANKLINE>
    '''
    def __init__(self, size):
        self.head = None
        self.maxSize = size
        self.remainingSpace = size
        self.numItems = 0

    def __str__(self):
        listString = ""
        current = self.head
        while current is not None:
            listString += "[" + str(current.value) + "]\n"
            current = current.next
        return 'REMAINING SPACE:{}\nITEMS:{}\nLIST:\n{}'.format(self.remainingSpace, self.numItems, listString)

    __repr__=__str__

    def __len__(self):
        return self.numItems
        
        
    def insert_head(self,node):
    #insert node as head
        node.next = self.head
        self.head = node
        
        
    def delete_node(self, content):
    #delete the node which has given content
        if self.head.value == content:
            temp = self.head.next
            self.head.next = None
            self.head = temp
        else:
            current = self.head
            while current.next.value != content:
                current = current.next
            temp = current.next.next
            current.next.next = None
            current.next = temp
    
    
    def find_content(self, cid):
    #find the content of given cid
        current = self.head
        while current != None:
            if current.value.cid == cid:
                return current.value
            current = current.next
        return None
        
        
    def put(self, content, evictionPolicy):
        # YOUR CODE STARTS HERE
        if content.size > self.maxSize:
            return "Insertion not allowed"
        elif self.__contains__(content.cid) == True:
            self.delete_node(content)
            self.insert_head(Node(content))
            return f"INSERTED: {content}"
        else:
            remain = self.remainingSpace - content.size
            while remain < 0:
                if evictionPolicy == 'mru':
                    self.mruEvict()
                    remain = self.remainingSpace - content.size
                elif evictionPolicy == 'lru':
                    self.lruEvict()
                    remain = self.remainingSpace - content.size
            node = Node(content)
            self.insert_head(node)
            self.remainingSpace = remain
            self.numItems += 1
            return f"INSERTED: {content}"
                
                
    def __contains__(self, cid):
        # YOUR CODE STARTS HERE
        if self.find_content(cid) == None:
            return False
        temp = self.find_content(cid)
        self.delete_node(temp)
        self.insert_head(Node(temp))
        return True
            


    def update(self, cid, content):
        # YOUR CODE STARTS HERE
        if self.__contains__(cid) == False:
            return "Cache miss!"
        else:
            temp = self.find_content(cid)
            remain = self.remainingSpace + temp.size - content.size
            if remain < 0:
                return "Cache miss!"
            else:
                self.delete_node(temp)
                self.insert_head(Node(content))
                self.remainingSpace = remain
                return f"UPDATED: {content}"
           
    
    def mruEvict(self):
        # YOUR CODE STARTS HERE
        self.remainingSpace += self.head.value.size
        self.numItems -= 1
        temp = self.head.next
        self.head.next = None
        self.head = temp
        

    def lruEvict(self):
        # YOUR CODE STARTS HERE
        if self.head.next == None:
            self.remainingSpace += self.head.value.size
            self.head = None
        else:
            current = self.head
            while current.next.next != None:
                current = current.next
            self.remainingSpace += current.next.value.size
            current.next = None
        self.numItems -= 1

    
    def clear(self):
        # YOUR CODE STARTS HERE
        self.head.next = None
        self.head = None
        self.remainingSpace = self.maxSize
        self.numItems = 0
        return "Cleared cache!"
        

class Cache:
    """
        # An extended version available on Canvas. Make sure you pass this doctest first before running the extended version

        >>> content1 = ContentItem(1000, 10, "Content-Type: 0", "0xA")
        >>> content2 = ContentItem(1004, 50, "Content-Type: 1", "110010")
        >>> content3 = ContentItem(1005, 180, "Content-Type: 2", "<html><p>'CMPSC132'</p></html>")
        >>> content4 = ContentItem(1006, 18, "another header", "111110")
        >>> content5 = ContentItem(1008, 2, "items", "11x1110")
        >>> lst=CacheList(200)
        >>> lst
        REMAINING SPACE:200
        ITEMS:0
        LIST:
        <BLANKLINE>
        >>> lst.put(content1, 'mru')
        'INSERTED: CONTENT ID: 1000 SIZE: 10 HEADER: Content-Type: 0 CONTENT: 0xA'
        >>> lst.put(content2, 'lru')
        'INSERTED: CONTENT ID: 1004 SIZE: 50 HEADER: Content-Type: 1 CONTENT: 110010'
        >>> lst.put(content4, 'mru')
        'INSERTED: CONTENT ID: 1006 SIZE: 18 HEADER: another header CONTENT: 111110'
        >>> lst.put(content5, 'mru')
        'INSERTED: CONTENT ID: 1008 SIZE: 2 HEADER: items CONTENT: 11x1110'
        >>> lst.put(content3, 'lru')
        "INSERTED: CONTENT ID: 1005 SIZE: 180 HEADER: Content-Type: 2 CONTENT: <html><p>'CMPSC132'</p></html>"
        >>> lst.put(content1, 'mru')
        'INSERTED: CONTENT ID: 1000 SIZE: 10 HEADER: Content-Type: 0 CONTENT: 0xA'
        >>> 1006 in lst
        True
        >>> contentExtra = ContentItem(1034, 2, "items", "other content")
        >>> lst.update(1008, contentExtra)
        'UPDATED: CONTENT ID: 1034 SIZE: 2 HEADER: items CONTENT: other content'
        >>> lst
        REMAINING SPACE:170
        ITEMS:3
        LIST:
        [CONTENT ID: 1034 SIZE: 2 HEADER: items CONTENT: other content]
        [CONTENT ID: 1006 SIZE: 18 HEADER: another header CONTENT: 111110]
        [CONTENT ID: 1000 SIZE: 10 HEADER: Content-Type: 0 CONTENT: 0xA]
        <BLANKLINE>
        >>> lst.clear()
        'Cleared cache!'
        >>> lst
        REMAINING SPACE:200
        ITEMS:0
        LIST:
        <BLANKLINE>
    """

    def __init__(self):
        self.hierarchy = [CacheList(200), CacheList(200), CacheList(200)]
        self.size = 3
    
    def __str__(self):
        return ('L1 CACHE:\n{}\nL2 CACHE:\n{}\nL3 CACHE:\n{}\n'.format(self.hierarchy[0], self.hierarchy[1], self.hierarchy[2]))
    
    __repr__=__str__


    def clear(self):
        for item in self.hierarchy:
            item.clear()
        return 'Cache cleared!'

    
    def insert(self, content, evictionPolicy):
        # YOUR CODE STARTS HERE
        if content.cid in self.hierarchy[hash(content)]:
            self.hierarchy[hash(content)].put(content, evictionPolicy)
            return f"Content {content.cid} already in cache, insertion not allowed"
        return self.hierarchy[hash(content)].put(content, evictionPolicy)


    def __getitem__(self, content):
        # YOUR CODE STARTS HERE
        if content.cid in self.hierarchy[hash(content)]:
            return content
        return "Cache miss!"



    def updateContent(self, content):
        # YOUR CODE STARTS HERE
        return self.hierarchy[hash(content)].update(content.cid,content)

if __name__ == "__main__":
    import doctest
    doctest.testmod()
   

