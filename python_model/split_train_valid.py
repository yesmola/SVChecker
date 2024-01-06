import os
import random

def main():
    # for type in ["AccessControl", "DoS", "IntOverflow", "Reentrancy", "UncheckedCall"]:
    for index, type in enumerate(["AccessControl", "DoS", "IntOverflow", "Reentrancy", "UncheckedCall"]):
        for value in ["secure", "vulnerable"]:
            path = "/home/yuanye/graduation/BugDataset_new/" + type + "/" + value + "/"
            filenames = os.listdir(path)
            # print(type, value, len(filenames))
            num = min(len(filenames), 100000)
            random.shuffle(filenames)
            train_filenames = filenames[ : int(0.1*num)]
            with open('./test.txt', 'a') as f:
                for train_filename in train_filenames:
                    f.write(path + train_filename + " " + ("0\n" if value == "secure" else str(index + 1) + '\n'))
            # train_filenames = filenames[ : int(0.8*num)]
            # valid_filenames = filenames[int(0.8*num) : num - 1]
            # word_num = 0
            # max_word_num = 0
            # min_word_num = 10000
            # for filename in filenames:
            #     with open(path + filename, 'r') as f:
            #         data = f.read()
            #         num = len(data.split(' '))
            #         max_word_num = max(max_word_num, num)
            #         min_word_num = min(min_word_num, num)
            #         word_num += num
            # print(type, value, max_word_num, min_word_num, word_num / len(filenames))
            # with open('./train_set.txt', 'a') as f:
            #     for train_filename in train_filenames:
            #         f.write(path + train_filename + " " + ("0\n" if value == "secure" else str(index + 1) + '\n'))
            # with open('./valid_set.txt', 'a') as f:
            #     for valid_filename in valid_filenames:
            #         f.write(path + valid_filename + " " + ("0\n" if value == "secure" else str(index + 1) + '\n'))
    return

if __name__ == "__main__":
    main()