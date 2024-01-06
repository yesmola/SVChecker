import torch
from torch.utils.data import Dataset

from vectorize_patch import PatchVectorizer

class CustomDataset(Dataset):
    def __init__(self, cases, config):
        self.cases = cases
        self.config = config
        self.vectorizer = PatchVectorizer(self.config)
        self.vectorizer.load_model()
            
    def __len__(self):
        return len(self.cases)

    def __getitem__(self, index):
        code_path = self.cases[index].split(' ')[0]
        label = self.cases[index].split(' ')[1]
        
        with open(code_path, encoding='utf-8') as f:
            code_snippet = f.read()

        vector = self.vectorizer.vectorize(code_snippet)
        # 将图像和标签转换为张量
        vector = torch.from_numpy(vector).to(torch.float32).to(self.config.device)
        label = torch.tensor(int(label)).to(self.config.device)
        return vector, label
