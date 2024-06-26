############################################################
# CMPSC442: Classification
############################################################

student_name = "Xuhong Lin"

############################################################
# Imports
############################################################

# Include your imports here, if any are used.
import email
import math
import os

############################################################
# Section 1: Spam Filter
############################################################

def load_tokens(email_path):
    '''
    >>> ham_dir = "homework5_data/train/ham/"
    >>> load_tokens(ham_dir+"ham1")[200:204]
    ['of', 'my', 'outstanding', 'mail']
    >>> load_tokens(ham_dir+"ham2")[110:114]
    ['for', 'Preferences', '-', "didn't"]
    >>> spam_dir = "homework5_data/train/spam/"
    >>> load_tokens(spam_dir+"spam1")[1:5]
    ['You', 'are', 'receiving', 'this']
    >>> load_tokens(spam_dir+"spam2")[:4]
    ['<html>', '<body>', '<center>', '<h3>']
    '''
    words = []
    fd = open(email_path, 'r')
    message = email.message_from_file(fd)
    for lines in email.iterators.body_line_iterator(message):
        words += lines.split()
    fd.close()
    return words

def log_probs_helper(email_paths):
    # return dict = {word: counts}, sum(all words' counts), len(dict)
    dict = {}
    for email_path in email_paths:
        # get a list of words in this path
        words = load_tokens(email_path)
        # build dict
        for word in words:
            if word not in dict:
                dict[word] = 1
            else:
                dict[word] = dict[word] + 1
    return dict, sum(count for count in dict.values()), len(dict)
        
def log_probs(email_paths, smoothing):
    '''
    >>> paths = ["homework5_data/train/ham/ham%d" % i
    ... for i in range(1, 11)]
    >>> p = log_probs(paths, 1e-5)
    >>> p["the"]
    -3.6080194731874062
    >>> p["line"]
    -4.272995709320345
    >>> paths = ["homework5_data/train/spam/spam%d" % i
    ... for i in range(1, 11)]
    >>> p = log_probs(paths, 1e-5)
    >>> p["Credit"]
    -5.837004641921745
    >>> p["<UNK>"]
    -20.34566288044584
    '''
    dict, sum_vocab_counts, vocab_num = log_probs_helper(email_paths)
    log_probs_dict = {}
    for word in dict.keys():
        # log(P(w)) = log((count(w) + a) / (sum(all vocabuary counts) + a * (vocab_num + 1)))
        log_probs_dict[word] = math.log((dict[word] + smoothing) / (sum_vocab_counts + smoothing * (vocab_num + 1)))
    # consider the special word by put "<UNK>" as key
    # log(P("<UNK>")) = log(a / (sum(all vocabuary counts) + a * (vocab_num + 1)))
    log_probs_dict["<UNK>"] = math.log(smoothing / (sum_vocab_counts + smoothing * (vocab_num + 1)))
    return log_probs_dict
    
class SpamFilter(object):
    '''
    >>> sf = SpamFilter("homework5_data/train/spam",
    ... "homework5_data/train/ham", 1e-5)
    >>> sf.is_spam("homework5_data/train/spam/spam1")
    True
    >>> sf.is_spam("homework5_data/train/spam/spam2")
    True
    >>> sf = SpamFilter("homework5_data/train/spam",
    ... "homework5_data/train/ham", 1e-5)
    >>> sf.is_spam("homework5_data/train/ham/ham1")
    False
    >>> sf.is_spam("homework5_data/train/ham/ham2")
    False
    '''
    def __init__(self, spam_dir, ham_dir, smoothing):
        # get all email paths
        spam_emails = os.listdir(spam_dir)
        ham_emails = os.listdir(ham_dir)
        spam_paths = []
        ham_paths = []
        for i in spam_emails:
            spam_paths.append(spam_dir + "/" + i)
        for j in ham_emails:
            ham_paths.append(ham_dir + "/" + j)
     
        self.spam_dict = log_probs(spam_paths, smoothing)
        self.ham_dict = log_probs(ham_paths, smoothing)
        self.spam_prob = len(spam_paths) / (len(spam_paths) + len(ham_paths))
        self.ham_prob = len(ham_paths) / (len(spam_paths) + len(ham_paths))
    
    def is_spam(self, email_path):
        '''
        >>> sf = SpamFilter("homework5_data/train/spam",
        ... "homework5_data/train/ham", 1e-5)
        >>> sf.is_spam("homework5_data/train/spam/spam1")
        True
        >>> sf.is_spam("homework5_data/train/spam/spam2")
        True
        >>> sf = SpamFilter("homework5_data/train/spam",
        ... "homework5_data/train/ham", 1e-5)
        >>> sf.is_spam("homework5_data/train/ham/ham1")
        False
        >>> sf.is_spam("homework5_data/train/ham/ham2")
        False
        '''
        # class c = {spam, ~spam}
        # P(c|document) = P(c)* ∏(P(w|c)^count(w))for w in v
        # since we will take log of the result to avoid underflow
        # therefore, we can code each log of arguments and add them up, log(a*b) = log(a)+log(b)
        # note that P(w|c)^count(w) is calcualted in log_prob_helper function, and all probility are stored in self.spam_dict, self.ham_dict
        words_dict,_,_ = log_probs_helper([email_path])  # get the dictionary of each word and its counts in this email path
        total_spam_prob = math.log(self.spam_prob) # log(P(c = spam))
        total_ham_prob = math.log(self.ham_prob) # log(P(c = ~spam))
        for word in words_dict:
            if word in self.spam_dict:
                total_spam_prob += self.spam_dict[word] 
            else: # word does not find in dict, put <UNK> to avoid zero probability
                total_spam_prob += self.spam_dict["<UNK>"]
            if word in self.ham_dict:
                total_ham_prob += self.ham_dict[word]
            else: # word does not find in dict, put <UNK> to avoid zero probability
                total_ham_prob += self.ham_dict["<UNK>"]
        
        # check if spam
        if total_spam_prob >= total_ham_prob:
            return True
        else:
            return False

    def most_indictive_helper(self, n, category):
        # log(P(w|spam)/P(w)) = log(P(w|spam) - log(P(w)) and log(P(w|~spam)/P(w)) = log(P(w|~spam) - log(P(w))
        # we already have log(P(w|spam)) and log(P(w|~spam)) corresponding in the self.spam_dict[w] and self.ham_dict[w] 
        # Then, we know that P(w) = P(w|spam) + P(w|~spam)
        # Then, log(P(w)) = log(P(w|spam) + P(w|~spam)) = log(e^(log(P(w|spam))) + e^(log(P(w|~spam))))
        output = {}
        # check for the category
        if category == "ham":
            x = self.spam_dict #log(P(w|~spam))
            y = self.ham_dict
        elif category == "spam":
            x = self.ham_dict #log(P(w|spam))
            y = self.spam_dict
        # loop for each word
        for word in x:
            if word in y:
                word_prob = math.exp(x[word]) + math.exp(y[word]) # P(w) = e^(log(P(w|spam))) + e^(log(P(w|~spam))
            else:
                word_prob = math.exp(x[word]) + math.exp(y["<UNK>"])
            output[word] = x[word] - math.log(word_prob)
        # rearrange the output in decending order
        sorted_output = sorted(output.items(), key=lambda x: x[1], reverse=False)
        return [word for word, _ in sorted_output][:n]

    def most_indicative_spam(self, n):
        '''
        >>> sf = SpamFilter("homework5_data/train/spam",
        ... "homework5_data/train/ham", 1e-5)
        >>> sf.most_indicative_spam(5)
        ['<a', '<input', '<html>', '<meta',
        '</head>']
        '''
        return self.most_indictive_helper(n, "spam")

    def most_indicative_ham(self, n):
        '''
        >>> sf = SpamFilter("homework5_data/train/spam",
        ... "homework5_data/train/ham", 1e-5)
        >>> sf.most_indicative_ham(5)
        ['Aug', 'ilug@linux.ie', 'install',
        'spam.', 'Group:']
        '''
        return self.most_indictive_helper(n, "ham")


if __name__ == "__main__":
    import doctest
    doctest.testmod()