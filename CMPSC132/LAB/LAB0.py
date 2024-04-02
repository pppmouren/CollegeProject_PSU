#Lab
#0#Due Date: 08/28/2021, 11:59PM
# More information on pass statement:
def sumSquares(aList):
# --- YOU CODE STARTS HERE
    sum_Square = 0
    if isinstance(aList, list):
        new_list = []
        for i in aList:
            if type(i) == int or type(i) == float:
                new_list.append(i)
        for ele in new_list:
            if ele > 5 and ele < 500:
                sum_Square += ele ** 2
    else:
        sum_Square = None
    return sum_Square





# Uncomment next 3 lines if not running doctest in the command line
#if __name__ == "__main__":
#    import doctest
#    doctest.testmod()
