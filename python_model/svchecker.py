import warnings
warnings.filterwarnings("ignore")

import logging
import sys
import time
import pandas
import random
import torch
import numpy as np
import torch.utils.data as Data
from sklearn import metrics
from tensorboardX import SummaryWriter

from transformer_class import Config, TransformerModel
from transformer_class import train, test_single_file, test
from vectorize_patch import PatchVectorizer
from dataset import CustomDataset

class SVChecker:
    def __init__(self, config : Config):
        self.config = config
        self.code_snippets = []
        self.vectorizer = PatchVectorizer(self.config)
        self.vectors = []
        return

    def train_word2vec(self):
        logging.info("Train wrod2vec ...")
        f = open(self.config.train_set)
        lines = f.readlines()
        f.close()
        f = open(self.config.valid_set)
        lines += f.readlines()
        f.close()
        
        snippet_num = 0
        for line in lines:
            with open(line.split(' ')[0], encoding='utf-8') as f:
                # print(f.name)
                code_snippet = f.read()
            snippet_num += 1
            print("Loading file...", snippet_num, end="\r")
            self.vectorizer.add_patch(code_snippet)
        self.vectorizer.train_model()
        logging.info("Train wrod2vec successfully!")
    
    def train_transformer(self):
        logging.info("Start training transformer model...")
        f = open(self.config.train_set, 'r')
        train_cases = f.readlines()
        f.close()
        
        train_dataset = CustomDataset(train_cases, self.config)
        
        f = open(self.config.valid_set, 'r')
        valid_cases = f.readlines()
        f.close()
        
        valid_dataset = CustomDataset(valid_cases, self.config)
        
        train_iter = Data.DataLoader(dataset = train_dataset, batch_size = self.config.batch_size, shuffle=True)
        valid_iter = Data.DataLoader(dataset = valid_dataset, batch_size = self.config.batch_size, shuffle=True)
        # train
        model = TransformerModel(self.config).to(self.config.device)
        train(self.config, model, train_iter, valid_iter)
        logging.info("Train transformer model successfully")
        
    def test_case(self):
        model = TransformerModel(self.config).to(self.config.device)
        model.load_state_dict(torch.load(self.config.save_path))
        
        f = open(self.config.test_set, 'r')
        test_cases = f.readlines()
        f.close()
        random.shuffle(test_cases)
        test_dataset = CustomDataset(test_cases[:int(0.5 * len(test_cases))], self.config)
        test_iter = Data.DataLoader(dataset = test_dataset, batch_size = 1, shuffle=False)
        # print(test_single_file(self.config, model, test_iter))
        test(self.config, model, test_iter)
        return