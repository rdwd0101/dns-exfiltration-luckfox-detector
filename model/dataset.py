import os
from math import log
from collections import Counter
import torch
import pandas as pd
import numpy as np
from torch.utils.data import Dataset, DataLoader


class DNSExfiltrationDataset(Dataset):
    def __init__(self, data_frame, mean, std):
        self.data_frame = data_frame
        self.mean = mean
        self.std = std
    
    def __len__(self):
        return len(self.data_frame)

    def __getitem__(self, idx):
        if torch.is_tensor(idx):
            idx = idx.item()

        features = self.data_frame.iloc[idx, :4]
        gt = self.data_frame.iat[idx, 4]
        
        x_tensor = torch.tensor((features - self.mean) / self.std, dtype=torch.float32) # z-score normalization
        y_tensor = torch.tensor(int(gt), dtype=torch.long)
        
        return x_tensor, y_tensor
