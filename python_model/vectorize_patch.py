import os
import warnings
warnings.filterwarnings("ignore")

from gensim.models import Word2Vec
import numpy

# Sets for operators
operators2 = {
    '->', '++', '--', 
    '!~', '<<', '>>', '<=', '>=', 
    '==', '!=', '&&', '||', '+=', 
    '-=', '*=', '/=', '%=', '&=', '^=', '|=',
    '=>'
    }
operators1 = { 
    '(', ')', '[', ']', '.', 
    '+', '-', '*', '&', '/', 
    '%', '<', '>', '^', '|', 
    '=', ',', '?', ':' , ';',
    '{', '}'
    }

class PatchVectorizer:
    def __init__(self, config):
        self.patches = []
        self.vector_length = config.dim_model
        self.pad_size = config.pad_size
    
    @staticmethod
    def tokenize(line):
        tmp, w = [], []
        i = 0
        while i < len(line):
            # Ignore spaces and combine previously collected chars to form words
            if line[i] == ' ':
                tmp.append(''.join(w))
                tmp.append(line[i])
                w = []
                i += 1
            # Check operators and append to final list
            elif line[i:i+2] in operators2:
                tmp.append(''.join(w))
                tmp.append(line[i:i+2])
                w = []
                i += 2
            elif line[i] in operators1:
                tmp.append(''.join(w))
                tmp.append(line[i])
                w = []
                i += 1
            # Character appended to word list
            else:
                w.append(line[i])
                i += 1
        # Filter out irrelevant strings
        res = list(filter(lambda c: c != '', tmp))
        return list(filter(lambda c: c != ' ', res))

    @staticmethod
    def tokenize_patch(patch):
        tokenized = []
        for line in patch:
            tokens = PatchVectorizer.tokenize(line)
            tokenized += tokens
        return tokenized

    def add_patch(self, patch):
        tokenized_patch = PatchVectorizer.tokenize_patch(patch)
        self.patches.append(tokenized_patch)

    def vectorize(self, patch):
        tokenized_patch = PatchVectorizer.tokenize_patch(patch)
        vectors = numpy.zeros(shape=(self.pad_size, self.vector_length))
        for i in range(min(len(tokenized_patch), self.pad_size)):
            vectors[i] = self.embedding[tokenized_patch[i]]
        return vectors

    def train_model(self):
        model = Word2Vec(self.patches, min_count=1, vector_size=self.vector_length, sg=1)
        model.save("word2vec.model")
        self.embedding = model.wv
        del model
        del self.patches
        
    def load_model(self):
        if os.path.exists("./uc/word2vec.model"):
            model = Word2Vec.load("./uc/word2vec.model")
            self.embedding = model.wv
            del model
            return True
        return False