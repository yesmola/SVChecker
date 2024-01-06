import warnings
warnings.filterwarnings("ignore")

import logging
import torch
import os

from svchecker import SVChecker
from transformer_class import Config

def main():
    config = Config()
    svc = SVChecker(config)
    # if not os.path.exists("word2vec.model"):
    #     svc.train_word2vec()
    # svc.train_transformer()
    svc.test_case()
        

if __name__ == "__main__":
    logging.basicConfig(format='%(asctime)s - %(filename)s[line:%(lineno)d] - %(levelname)s: %(message)s',
                        level=logging.DEBUG)
    main()