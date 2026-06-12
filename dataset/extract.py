import os
import pandas as pd
import numpy as np
from features import extract_features

path = "./training.csv"
df = pd.read_csv(path, header=None)

rows = []
for idx, row in df.iterrows():
    feat = extract_features(row.iloc[1])
    rows.append(feat.tolist() + [row.iloc[0]])

output_path = "features.csv"
feat_df = pd.DataFrame(rows)
feat_df.to_csv(output_path, index=False, header=False)

print("Successfully saved dataset to {}".format(output_path))

features = feat_df.iloc[:, :4].values.astype(np.float32)
mean = features.mean(axis=0)
std  = features.std(axis=0) + 1e-8  # to avoid division-by-zero

scaler_params_path = "training_scaler.npz"
np.savez(scaler_params_path, mean=mean, std=std)

print("Successfully saved mean={} and std={} to {}".format(mean, std, scaler_params_path))
os.makedirs('quant_samples', exist_ok=True)
paths = []

for i, row in feat_df.iterrows():
    feat = row.iloc[1:].values.astype(np.float32)
    feat = (feat - mean) / std   # normalize here
    feat = feat.reshape(1, -1)
    path = f'quant_samples/sample_{i}.npy'
    np.save(path, feat)
    paths.append(os.path.abspath(path))

with open('quant_dataset.txt', 'w') as f:
    f.write('\n'.join(paths))

