# LAB3
# Due Date: 10/01/2021, 11:59PM
# REMINDERS:
#        The work in this assignment must be your own original work and must be completed alone.
#        All functions should NOT contain any for/while loops or global variables. Use recursion, otherwise no credit will be given

def get_count(aList, item):
    '''
        >>> get_count([1,4,3.5,'1',3.5, 9, 1, 4, 2], 1)
        2
        >>> get_count([1,4,3.5,'1',3.5, 9, 4, 2], 3.5)
        2
        >>> get_count([1,4,3.5,'1',3.5, 9, 4, 2], 9)
        1
        >>> get_count([1,4,3.5,'1',3.5, 9, 4, 2], 'a')
        0
    '''
    ## YOUR CODE STARTS HERE
    i = 0
    if len(aList) == 0:
        return i
    else:
        if aList[len(aList)-1] == item:
            i = 1
        return i + get_count(aList[:-1], item)



def replace(numList, old, new):
    '''
        >>> input_list = [1, 7, 5.6, 3, 2, 4, 1, 9]
        >>> replace(input_list, 1, 99.9)
        [99.9, 7, 5.6, 3, 2, 4, 99.9, 9]
        >>> input_list
        [1, 7, 5.6, 3, 2, 4, 1, 9]
        >>> replace([1,7, 5.6, 3, 2, 4, 1, 9], 5.6, 777)
        [1, 7, 777, 3, 2, 4, 1, 9]
        >>> replace([1,7, 5.6, 3, 2, 4, 1, 9], 8, 99)
        [1, 7, 5.6, 3, 2, 4, 1, 9]
    '''
    ## YOUR CODE STARTS HERE
    lst = []
    if len(numList) == 1:
        if numList[0] == old:
            lst.append(new)
            return lst
        else:
            lst.append(numList[0])
            return lst
    else:
        if  numList[len(numList)-1]== old:
            lst.append(new)
            return replace(numList[:-1], old, new) + lst
        else:
            lst.append(numList[len(numList)-1])
            return replace(numList[:-1], old, new) + lst

        


def flat(aList):
    '''
        >>> x = [3, [[5, 2]], 6, [4]]
        >>> flat(x)
        [3, 5, 2, 6, 4]
        >>> x
        [3, [[5, 2]], 6, [4]]
        >>> flat([1, 2, 3])
        [1, 2, 3]
        >>> flat([1, [], 3])
        [1, 3]
    '''
    ## YOUR CODE STARTS HERE
    if len(aList) == 0:
        return []
    else:
        if isinstance(aList[0],list):
            return flat(aList[0]) + flat(aList[1:])
        else:
            return [aList[0]] + flat(aList[1:])
            


def neighbor(n):
    """
        >>> neighbor(24680)
        24680
        >>> neighbor(2222466666678)
        24678
        >>> neighbor(0)
        0
        >>> neighbor(22224666666782)
        246782
        >>> neighbor(2222466666625)
        24625
    """
    ## YOUR CODE STARTS HERE
    if n == 0:
        return n
    else:
        digit1 = n % 10
        digit2 = n // 10
        digit3 = digit2 % 10
        if digit1 != digit3:
            return neighbor(digit2) * 10 + digit1
        else:
            return neighbor(digit2)
if __name__ == "__main__":
    import doctest
    doctest.testmod()
    

