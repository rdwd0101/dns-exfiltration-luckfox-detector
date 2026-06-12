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
        queries = self.data_frame.iloc[:, 1].values
        features = np.array([self.extract_features(q) for q in queries])
        self.mean = features.mean(axis=0)
        self.std  = features.std(axis=0) + 1e-8  # to avoid division-by-zero
        self.scaler_params_path = "{}scaler.npz".format(csv_file)
        np.savez(self.scaler_params_path, mean=self.mean, std=self.std)
        print("Successfully saved mean={} and std={} to {}".format(self.mean, self.std, self.scaler_params_path))

    def extract_features(self, query):
        features = [
            float(length(query)),
            float(shannon_entropy(query)),
            float(digit_ratio(query)),
            float(uppercase_ratio(query))
        ]
        return np.array(features)
    
    def __len__(self):
        return len(self.data_frame)

    def __getitem__(self, idx):
        if torch.is_tensor(idx):
            idx = idx.item()

        dns_string = self.data_frame.iat[idx, 1]
        gt = self.data_frame.iat[idx, 0]

        dns_string = "" if pd.isna(dns_string) else str(dns_string)

        features = self.extract_features(dns_string)
        x_tensor = torch.tensor((features - self.mean) / self.std, dtype=torch.float32) # z-score normalization
        y_tensor = torch.tensor(int(gt), dtype=torch.long)   # <- return tensor labels
        return x_tensor, y_tensor
