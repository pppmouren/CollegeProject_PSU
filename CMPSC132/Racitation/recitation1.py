def translate(mydict, text):
    text1 = text.lower()
    text2 = text1.split(' ')
    tra = []
    for i in text2:
        word = mydict.get(i)
        if word != None:
            tra.append(word)
        else:
            tra.append(i)
    return ' '.join(tra)
        
        
        


        
    
