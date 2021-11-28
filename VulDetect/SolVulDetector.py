import warnings
warnings.filterwarnings("ignore")

import sys
import os
import pandas
from gensim.models import Word2Vec
import numpy as np
import torch
import torch.utils.data as Data

from vectorize_patch import PatchVectorizer
from svm_clf import SVM
from transformer_class import Config
from transformer_class import TransformerModel
from transformer_class import train, test_single_file, test

import warnings
warnings.filterwarnings("ignore")

def parse_file(filename):
    with open(filename, "r", encoding='utf-8') as file:
        patch = []
        val = 0
        for line in file:
            # print(line)
            stripped = line.strip()
            if not stripped:
                continue
            if "-" * 40 in line: 
                yield patch, val
                patch = []
            elif stripped.split('.')[-1] == 'sol':
                continue
            elif stripped.split()[0].isdigit():
                if stripped.isdigit():
                    val = int(stripped)
                else:
                    patch.append(" ".join(stripped.split()[1:]))

def get_vectors(train_file, test_file, config):
    train_patch = []
    test_patch = []
    count = 0
    vectorizer = PatchVectorizer(config)
    for patch, val in parse_file(train_file):
        count += 1
        print("Collecting train patches...", count, end="\r")
        vectorizer.add_patch(patch)
        row = {"patch" : patch, "val" : val}
        train_patch.append(row)
    print()
    count = 0
    for patch, val in parse_file(test_file):
        count += 1
        print("Collecting test patches...", count, end="\r")
        vectorizer.add_patch(patch)
        row = {"patch" : patch, "val" : val}
        test_patch.append(row)
    print()
    print("Training model...")
    vectorizer.train_model()
    print()
    print("Turn to vector...")
    vectors = []
    count = 0
    for patch in train_patch:
        count += 1
        print("Processing train patches...", count, end="\r")
        vector = vectorizer.vectorize(patch["patch"])
        row = {"vector" : vector, "val" : patch["val"]}
        vectors.append(row)
    print()
    df = pandas.DataFrame(vectors)
    vectors = []
    count = 0
    for patch in test_patch:
        count += 1
        print("Processing test patches...", count, end="\r")
        vector = vectorizer.vectorize(patch["patch"])
        row = {"vector" : vector, "val" : patch["val"]}
        vectors.append(row)
    print()
    df_test = pandas.DataFrame(vectors)
    return df, df_test

def train_word2vec(train_file, test_dir, config):
    train_patch = []
    count = 0
    vectorizer = PatchVectorizer(config)
    for patch, val in parse_file(train_file):
        count += 1
        print("Collecting train patches...", count, end="\r")
        vectorizer.add_patch(patch)
        row = {"patch" : patch, "val" : val}
        train_patch.append(row)
    print()
    count = 0
    test_dict = {}
    for root, dir, test_files in os.walk(test_dir):
        for test_file in test_files:
            test_patch = []
            test_filepath = os.path.join(root, test_file)
            for patch, val in parse_file(test_filepath):
                count += 1
                print("Collecting patch file...", count, end="\r")
                vectorizer.add_patch(patch)
                row = {"patch" : patch, "val" : val}
                test_patch.append(row)
            test_dict[test_file] = test_patch
    print()
    print("Training model...")
    vectorizer.train_model()
    print()
    print("Generate vector file...")
    vectors = []
    count = 0
    for patch in train_patch:
        count += 1
        print("Processing train patches...", count, end="\r")
        vector = vectorizer.vectorize(patch["patch"])
        row = {"vector" : vector, "val" : patch["val"]}
        vectors.append(row)
    print()
    df = pandas.DataFrame(vectors)
    df.to_pickle("train_set_vectors.pkl")
    del df
    count = 0
    for filename in test_dict.keys():
        vectors = []
        test_patch = test_dict[filename]
        for patch in test_patch:
            count += 1
            print("Processing test patches...", count, end="\r")
            vector = vectorizer.vectorize(patch["patch"])
            row = {"vector" : vector, "val" : patch["val"]}
            vectors.append(row)
        df_test = pandas.DataFrame(vectors)
        base = os.path.splitext(os.path.basename(filename))[0]
        df_test.to_pickle("./test_pkl/" + base + ".pkl")
    print()
    return

def gen_sbcurated_vector(test_dir, config):
    count = 0
    test_dict = {}
    vectorizer = PatchVectorizer(config)
    for root, dir, test_files in os.walk(test_dir):
        for test_file in test_files:
            test_patch = []
            test_filepath = os.path.join(root, test_file)
            for patch, val in parse_file(test_filepath):
                count += 1
                print("Collecting patch file...", count, end="\r")
                vectorizer.add_patch(patch)
                row = {"patch" : patch, "val" : val}
                test_patch.append(row)
            test_dict[test_file] = test_patch
    print()
    vectorizer.train_model()
    count = 0
    for filename in test_dict.keys():
        vectors = []
        test_patch = test_dict[filename]
        for patch in test_patch:
            count += 1
            print("Processing test patches...", count, end="\r")
            vector = vectorizer.vectorize(patch["patch"])
            row = {"vector" : vector, "val" : patch["val"]}
            vectors.append(row)
        df_test = pandas.DataFrame(vectors)
        base = os.path.splitext(os.path.basename(filename))[0]
        df_test.to_pickle("./sbcurated/ul/" + base + ".pkl")
    print()

def svm_model(train_data, test_data):
    svm = SVM(train_data, test_data)
    svm.train()
    svm.test()

def train_torch(df,config):
    train_data = pandas.read_pickle(df)
    # data to tensor
    X_train = torch.from_numpy(np.stack(train_data.iloc[:, 0].values)).to(config.device).to(torch.float32)
    y_train = torch.from_numpy(train_data.iloc[:, 1].values).to(config.device).to(torch.long)
    print(X_train.shape, y_train.shape)
    # set train_iter and dev_iter
    train_set = Data.TensorDataset(X_train, y_train)
    train_size = int(0.8 * len(train_set))
    dev_size = len(train_set) - train_size
    train_dataset, vali_dataset = Data.random_split(train_set, [train_size, dev_size])
    train_iter = Data.DataLoader(dataset = train_dataset, batch_size = config.batch_size, shuffle=True)
    dev_iter = Data.DataLoader(dataset = vali_dataset, batch_size = config.batch_size, shuffle=True)
    # train
    print("Start to train")
    model = TransformerModel(config).to(config.device)
    train(config, model, train_iter, dev_iter)

def test_torch(test_pkl_dir, config):
    # ground_truth = {}
    # with open("../test_contracts.txt", "r", encoding="utf8") as file:
    #     for line in file:
    #         stripped = line.strip()
    #         name = stripped.split()[0]
    #         val = int(stripped.split()[1])
    #         ground_truth[name] = val

    model = TransformerModel(config).to(config.device)
    model.load_state_dict(torch.load(config.save_path))
    predicts = {}
    count = 0
    for root, dir, test_files in os.walk(test_pkl_dir):
        for test_file in test_files:
            count += 1
            print("Test source file...", count, end="\r")
            test_filepath = os.path.join(root, test_file)
            base = os.path.splitext(os.path.basename(test_file))[0]
            test_data = pandas.read_pickle(test_filepath)
            # print(len(test_data))
            if (len(test_data)) == 0:
                predicts[base] = 0
                continue
            X_test = torch.from_numpy(np.stack(test_data.iloc[:, 0].values)).to(config.device).to(torch.float32)
            y_test = torch.from_numpy(test_data.iloc[:, 1].values).to(config.device).to(torch.long)
            test_set = Data.TensorDataset(X_test, y_test)
            test_iter = Data.DataLoader(dataset = test_set, batch_size = 1, shuffle=True)
            label = test_single_file(config, model, test_iter)
            predicts[base] = label
    print()
    print(len(predicts))
    # compare ground truth and preditct
    tp = 0
    tn = 0
    fp = 0
    fn = 0
    for k in predicts.keys():
        if predicts[k] == 1:
            tp += 1
        if predicts[k] == 0:
            fn += 1
        # if ground_truth[k] == 1 and predicts[k] == 1:
        #     tp += 1
        # if ground_truth[k] == 1 and predicts[k] == 0:
        #     fn += 1
        # if ground_truth[k] == 0 and predicts[k] == 0:
        #     tn += 1
        # if ground_truth[k] == 0 and predicts[k] == 1:
        #     fp += 1
    print()
    print("true positive =", tp)
    print("true negative =", tn)
    print("false positive =", fp)
    print("false negative =", fn)
    print()
    # precision = tp / (tp + fp)
    # recall = tp / (tp + fn)
    # accuracy = (tp +  tn) / (tp + tn + fp + fn)
    # f1 = 2 / ((1 / precision) + (1 / recall))
    # print("Accuracy =", accuracy)
    # print("Precision =", precision)
    # print("Recall =", recall)
    # print("F1-score =", f1)

def torch_model(filename, test_pkl_dir, config):
    # data to tensor
    # X_test = torch.from_numpy(np.stack(test_data.iloc[:, 0].values)).to(config.device).to(torch.float32)
    # y_test = torch.from_numpy(test_data.iloc[:, 1].values).to(config.device).to(torch.long)
    # # set test_iter
    # test_set = Data.TensorDataset(X_test, y_test)
    # test_iter = Data.DataLoader(dataset = test_set, batch_size = config.batch_size, shuffle=True)

    # train
    # train_torch("train_set_vectors.pkl", config)
    test_torch(test_pkl_dir, config)

def torch_no_patch(train_data, test_data, config):
    # train_data = pandas.read_pickle(df)
    X_train = torch.from_numpy(np.stack(train_data.iloc[:, 0].values)).to(config.device).to(torch.float32)
    y_train = torch.from_numpy(train_data.iloc[:, 1].values).to(config.device).to(torch.long)
    print(X_train.shape, y_train.shape)
    # set train_iter and dev_iter
    train_set = Data.TensorDataset(X_train, y_train)
    train_size = int(0.8 * len(train_set))
    dev_size = len(train_set) - train_size
    train_dataset, vali_dataset = Data.random_split(train_set, [train_size, dev_size])
    train_iter = Data.DataLoader(dataset = train_dataset, batch_size = config.batch_size, shuffle=True)
    dev_iter = Data.DataLoader(dataset = vali_dataset, batch_size = config.batch_size, shuffle=True)
    # train
    print("Start to train")
    model = TransformerModel(config).to(config.device)
    train(config, model, train_iter, dev_iter)

    # test_data = pandas.read_pickle(df_test)
    X_test = torch.from_numpy(np.stack(test_data.iloc[:, 0].values)).to(config.device).to(torch.float32)
    y_test = torch.from_numpy(test_data.iloc[:, 1].values).to(config.device).to(torch.long)
    # set test_iter
    test_set = Data.TensorDataset(X_test, y_test)
    test_iter = Data.DataLoader(dataset = test_set, batch_size = config.batch_size, shuffle=True)
    test(config, model, test_iter)

def main():
    if len(sys.argv) != 3:
        print("Usage: python SolVulDetector.py [train_filename] [test_filename]")
        exit()
    config = Config()
    filename = sys.argv[1]
    test_dir = sys.argv[2]
    base = os.path.splitext(os.path.basename(filename))[0]
    # vector_filename = base + "_train_vectors64.pkl"
    # test_vector_filename = base + "_test_vectors64.pkl"
    # if os.path.exists(vector_filename) and os.path.exists(test_vector_filename):
    #     df = pandas.read_pickle(vector_filename)
    #     df_test = pandas.read_pickle(test_vector_filename)
    # else:
    #     df, df_test = get_vectors(filename, test_filename, config)
    #     df.to_pickle(vector_filename)
    #     df_test.to_pickle(test_vector_filename)
    
    # train_word2vec(filename, test_dir, config)
    # test_pkl_dir = "./test_pkl"
    # torch_model(filename, test_pkl_dir, config)
    # svm_model(df, df_test)

    ## 实验二： 对比不提取code patch
    # vector_filename = base + "_train_vectors64.pkl"
    # test_vector_filename = base + "_test_vectors64.pkl"
    # df, df_test = get_vectors(filename, test_dir, config)
    # df.to_pickle("vector_filename")
    # df_test.to_pickle(test_vector_filename)
    # #df = pandas.read_pickle(vector_filename)
    # #df_test = pandas.read_pickle(test_vector_filename)
    # torch_no_patch(df, df_test, config)

    ## 实验三： 对sbcurated进行测试
    gen_sbcurated_vector(test_dir, config)
    test_pkl_dir = "./sbcurated/ul/"
    torch_model(filename, test_pkl_dir, config)

if __name__ == "__main__":
    main()