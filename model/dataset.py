import os
from math import log
from collections import Counter
import torch
import pandas as pd
import numpy as np
from torch.utils.data import Dataset, DataLoader

def digit_ratio(query: str) -> float:
    return sum(c.isdigit() for c in query) / len(query)

def shannon_entropy(query: str) -> float:
    counts = Counter(query)
    frequencies = ((i / len(query)) for i in counts.values())
    return - sum(f * log(f, 2) for f in frequencies)

def length(query: str) -> int:
	return len(query)

def uppercase_ratio(query: str) -> float:
    return sum(c.isupper() for c in query) / len(query)

class DNSExfiltrationDataset(Dataset):
    def __init__(self, csv_file):
        """
        Arguments:
            csv_file (string): Path to the csv file
        """
        print("Opening {}".format(csv_file))
        self.data_frame = pd.read_csv(csv_file)

    def __len__(self):
        return len(self.data_frame)

    def __getitem__(self, idx):
        if torch.is_tensor(idx):
            idx = idx.item()

        dns_string = self.data_frame.iat[idx, 1]
        gt = self.data_frame.iat[idx, 0]

        dns_string = "" if pd.isna(dns_string) else str(dns_string)

        features = [
            float(length(dns_string)),
            float(shannon_entropy(dns_string)),
            float(digit_ratio(dns_string)),
            float(uppercase_ratio(dns_string))
        ]
        x_tensor = torch.tensor(features, dtype=torch.float32)
        y_tensor = torch.tensor(int(gt), dtype=torch.long)   # <- return tensor labels
        return x_tensor, y_tensor
