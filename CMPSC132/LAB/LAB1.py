# Lab #1
# Due Date: 09/03/2021, 11:59PM
# REMINDER: The work in this assignment must be your own original work and must be completed alone.


def joinList(n):
    '''
        >>> joinList(5)
        [1, 2, 3, 4, 5, 5, 4, 3, 2, 1]
        >>> joinList(1)
        [1, 1]
        >>> joinList(-3) is None
        True

    '''
    # - YOUR CODE STARTS HERE -
    lst1 = []
    if n > 0:
        for i in range(n):
            lst1.append(i + 1)
        lst2 = lst1[::-1]     #reverse the list
        for ele in lst2:
            lst1.append(ele)
    else:
        lst1 = None
    return lst1

def isValid(txt):
    '''
        >>> isValid('qwertyuiopASDFGHJKLzxcvbnm')
        True
        >>> isValid('hello there, fall is here!')
        False
        >>> isValid('123456yh')
        False
        >>> isValid('POIUYTqwerASDFGHlkjZXCVBMn')
        True
        >>> isValid('POIUYTqwerASDFGHlkjZXCVBnn')
        False
        >>> isValid('12aaaaaaaaaaa6543212345678')
        False
        >>> isValid([1]*26) is None
        True
    '''
    # - YOUR CODE STARTS HERE -
    is_Valid = True
    if type(txt) == str:
        for ele in txt:
            if ord('A') <= ord(ele) <= ord('Z') or ord('a') <= ord(ele) <= ord('z'):
                is_Valid = True
            else:
                is_Valid = False
                break
        for i in range(len(txt)):
            for j in range(i + 1, len(txt)):
                if i == j:
                    is_Valid = False
    else:
        is_Valid = None
    return is_Valid
                
        




def removePunctuation(aString):
    '''
        >>> removePunctuation("Dots...................... many dots..X")
        ('Dots                       many dots  X', {'.': 24})
        >>> data = removePunctuation("I like chocolate cake!!(!! It's the best flavor..;.$ for real")
        >>> data[0]
        'I like chocolate cake      It s the best flavor      for real'
        >>> data[1]
        {'!': 4, '(': 1, "'": 1, '.': 3, ';': 1, '$': 1}
        
    '''
    # - YOUR CODE STARTS HERE -
    punc = []
    my_dict = {}
    for i in aString:
        if i.isalpha() == False:
            aString = aString.replace(i,' ')
            if i != ' ':
                punc.append(i)
    punc.sort()
    '''
    for key in punc:
        if key not in my_dict:
            my_dict[key] = 1
        else:
            my_dict[key] += 1
    '''
    my_dict[punc[0]] = punc.count(punc[0])
    for ele in range(len(punc) - 1):
        if punc[ele] != punc[ele + 1]:
            my_dict[punc[ele + 1]] = punc.count(punc[ele + 1])
    return aString, my_dict
    
            
            
            
        
         
            




#if __name__ == "__main__":
#    import doctest
#    doctest.testmod()
