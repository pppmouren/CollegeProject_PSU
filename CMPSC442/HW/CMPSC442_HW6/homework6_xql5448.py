############################################################
# CMPSC 442: Hidden Markov Models
############################################################

student_name = "Xuhong Lin"

############################################################
# Imports
############################################################
# Include your imports here, if any are used.
from collections import defaultdict

############################################################
# Section 1: Hidden Markov Models
############################################################
def load_corpus(path):
    '''
    >>> c = load_corpus("brown-corpus.txt")
    >>> c[1402]
    [('It', 'PRON'), ('made', 'VERB'), ('him', 'PRON'), ('human', 'NOUN'), ('.', '.')]
    >>> c = load_corpus("brown-corpus.txt")
    >>> c[1799]
    [('The', 'DET'), ('prospects', 'NOUN'), ('look', 'VERB'), ('great', 'ADJ'), ('.', '.')]
    '''
    with open(path, 'r') as file:
        corpus = []
        for line in file:
            sentence = []
            for pair in line.split():
                sentence.append(tuple(pair.split("=")))
            corpus.append(sentence)
    return corpus
                

class Tagger(object):
    '''
    >>> c = load_corpus("brown-corpus.txt")
    >>> t = Tagger(c)
    >>> t.most_probable_tags(["The", "man", "walks", "."])
    ['DET', 'NOUN', 'VERB', '.']
    >>> c = load_corpus("brown-corpus.txt")
    >>> t = Tagger(c)
    >>> t.most_probable_tags(["The", "blue", "bird", "sings"])
    ['DET', 'ADJ', 'NOUN', 'VERB']
    >>> c = load_corpus("brown-corpus.txt")
    >>> t = Tagger(c)
    >>> s = "I am waiting to reply".split()
    >>> t.most_probable_tags(s)
    ['PRON', 'VERB', 'VERB', 'PRT', 'NOUN']
    >>> t.viterbi_tags(s)
    ['PRON', 'VERB', 'VERB', 'PRT', 'VERB']
    >>> c = load_corpus("brown-corpus.txt")
    >>> t = Tagger(c)
    >>> s = "I saw the play".split()
    >>> t.most_probable_tags(s)
    ['PRON', 'VERB', 'DET', 'VERB']
    >>> t.viterbi_tags(s)
    ['PRON', 'VERB', 'DET', 'NOUN']
    '''
    def __init__(self, sentences):
        self.all_tags = ['NOUN', 'VERB','ADJ', 'ADV', 'PRON', 'DET', 'ADP', 'NUM', 'CONJ', 'PRT', '.', 'X']
        self.tag_size = 12
        self.init_prob = {}
        self.transition_prob = {}
        self.emission_prob = {}
        
        init_count = defaultdict(int)        # {tags(start of the sentence) : appear times}
        transition_count = defaultdict(int)  # {(t_i, t_i+1) : appear times}
        emission_count = defaultdict(int)    # {(t_i, w_i) : appear_times}
        total_word_count = defaultdict(int)  # {w_i : appear times}
        total_tag_count = defaultdict(int)   # {t_i : appear times}
        # fill up those counts, 
        for sentence in sentences:
            init_count[sentence[0][1]] += 1
            for i in range(len(sentence)):
                if i < len(sentence) - 1:
                    transition_count[(sentence[i][1], sentence[i+1][1])] += 1
                    
                emission_count[(sentence[i][1], sentence[i][0])] += 1
                total_word_count[sentence[i][0]] += 1
                total_tag_count[sentence[i][1]] += 1
        # calculate the probability
        # initial probility
        for tag in init_count:
            init_prob = (init_count[tag] + 1) / (len(sentences) + len(init_count)) # Laplacian smoothing  
            self.init_prob[tag] = init_prob
            
        # transition probility 
        for (tag, following_tag) in transition_count:
            # p(t_i+1 | t_i) = (N(t_i+1, t_i) + 1) / (N(t_i) + |V|) 
            # (|V| is vocabulary size = size of vocab set that t_i+1 belongs to which is the size of tags)
            transition_prob = (transition_count[(tag, following_tag)] + 1) / (total_tag_count.get(tag) + self.tag_size)
            self.transition_prob[(tag, following_tag)] = transition_prob
        
        # emission probility
        for (tag, word) in emission_count:
            # p(w_i | t_i) = (N(w_i, t_i) + 1) / (N(t_i) + |V|) 
            # (|V| is size of words
            # note that below equation, total_word_count + 1, 1 is "<UNK>"
            emission_prob = (emission_count[(tag, word)] + 1) / (total_tag_count.get(tag) + len(total_word_count) + 1)
            self.emission_prob[(tag, word)] = emission_prob
        
        # consider all unseen t_i->t_i+1 as 'X'->t_i+1, and t_i->w_i as 'X'->w_i
        for t_next in total_tag_count.keys():
            if ('X', t_next) not in transition_count:
                #total_tag_count['X'] += 1
                transition_count[('X', t_next)] = 1 / (total_tag_count.get('X', 0) + len(self.all_tags))
        for w in total_word_count.keys():
            if ('X', w) not in emission_count:
                emission_count[('X', w)] = 1 / ((total_tag_count.get('X', 0) + len(total_word_count)))
            
    def most_probable_tags(self, tokens):
        tags_list = []
        for token in tokens:
            temp = []
            for tag in self.all_tags:
                if ((tag, token) in self.emission_prob.keys()):
                    temp.append((self.emission_prob.get((tag, token), 0), tag))
                else:
                    temp.append((self.emission_prob.get(("X", token), 0), "X"))
            tags_list.append(max(temp)[1])
        return tags_list

    def viterbi_tags(self, tokens):
        v = [{} for i in range(len(tokens))]
        bt = [{} for i in range(len(tokens))]
        # Initialization
        # v_1(j) = pi_j*b_j(o_1) 1<= j <= N (j is all tag types) 
        # bt_1(j) = 0 1<= j <= N
        for tag in self.all_tags:
            v[0][tag] = self.init_prob.get(tag, 0) * self.emission_prob.get((tag, tokens[0]), 0)
        
        # Recursion
        # v_t(j) = max(v_t-1(i)*a_ij*b_j(o_t))  1<= j <= N, 1<t<=T
        # bt_t(j) = argmax(v_t-1(i)*a_ij*b_j(o_t))   1<= j <= N, 1<t<=T
        for t in range(1, len(tokens)):
            for tag in self.all_tags: # tag is j
                max_prob = 0
                max_tag = None
                for prev_tag in self.all_tags: # prev_tag is i
                    transition_X_tag = self.transition_prob.get(('X', tag), 0.0000001)
                    emission_X_tag = self.emission_prob.get(('X', tokens[t]), 0.0000001)
                    temp_prob = v[t-1][prev_tag] * self.transition_prob.get((prev_tag, tag), transition_X_tag) * self.emission_prob.get((tag, tokens[t]), emission_X_tag)
                    if temp_prob > max_prob:
                        max_prob = temp_prob
                        max_tag = prev_tag
                v[t][tag] = max_prob
                bt[t][tag] = max_tag
                
        # Termination:
        # the best score: P* = max(v_T(i))
        # The start of backtrace: qT* = argmax(v_T(i))
        best_result_tags = []
        best_tag = None
        best_prob = 0
        # start from back, find the tag with the highest prob in the last column
        for tag in self.all_tags:
            if v[-1][tag] > best_prob:
                best_prob = v[-1][tag]
                best_tag = tag
        best_result_tags.append(best_tag)
        # Move forward
        for t in range(len(tokens) - 1, 0, -1):
            best_tag = bt[t][best_tag]
            best_result_tags.insert(0, best_tag)
        return best_result_tags
        
        
if __name__ == "__main__":
    import doctest
    doctest.testmod()