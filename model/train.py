import os
import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, TensorDataset
from ignite.engine import Engine, Events, create_supervised_trainer, create_supervised_evaluator
from ignite.metrics import Accuracy, Loss
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler


class BinaryClassifier(nn.Module):
    def __init__(self):
        super(BinaryClassifier, self).__init__()
        self.net = nn.Sequential(
            nn.Linear(4, 16),
            nn.ReLU(),
            nn.Linear(16, 8),
            nn.ReLU(),
            nn.Linear(8, 1)
        )        
        
    def forward(self, x):
        out = self.net(x)
        return out.view(-1)


scaler = StandardScaler()
device = "cuda" if torch.cuda.is_available() else "cpu"
model = BinaryClassifier().to(device)

parent_path = os.path.abspath(os.path.pardir)
train_path = os.path.join(parent_path, "dataset", "output", "features.csv")
val_path = os.path.join(parent_path, "dataset", "output", "features_val.csv")

df_train = pd.read_csv(train_path, header=None)
df_val = pd.read_csv(val_path, header=None)
df_all = pd.concat([df_train, df_val], ignore_index=True)

X = df_all.iloc[:, :4].values
y = df_all.iloc[:, 4].values

X_train, X_val, y_train, y_val = train_test_split(X, y, test_size=0.2, stratify=y, shuffle=True)

scaler = StandardScaler()
X_train_norm = scaler.fit_transform(X_train)
X_val_norm = scaler.transform(X_val)

train_df = pd.read_csv(train_path)
train_val = pd.read_csv(val_path)
train_features = torch.tensor(X_train_norm, dtype=torch.float32)
train_labels = torch.tensor(y_train, dtype=torch.float32)

val_features = torch.tensor(X_val_norm, dtype=torch.float32)
val_labels = torch.tensor(y_val, dtype=torch.float32)

train_dataset = TensorDataset(train_features, train_labels)
val_dataset = TensorDataset(val_features, val_labels)

BATCH_SIZE = 64

train_loader = DataLoader(train_dataset, batch_size=BATCH_SIZE, shuffle=True)
val_loader = DataLoader(val_dataset, batch_size=BATCH_SIZE, shuffle=False)

criterion = nn.BCEWithLogitsLoss()
optimizer = optim.Adam(model.parameters(), lr=1e-3)

def train_step(engine, batch):
    model.train()
    x, y = batch
    x = x.to(device)
    y = y.to(device).float()
    optimizer.zero_grad()
    logits = model(x)
    loss = criterion(logits, y)
    loss.backward()
    optimizer.step()
    return loss.item()

trainer = Engine(train_step)

def eval_step(engine, batch):
    model.eval()
    with torch.no_grad():
        x, y = batch
        x = x.to(device)
        y = y.to(device)
        logits = model(x)
        return logits, y

evaluator_train = Engine(eval_step)
evaluator_val = Engine(eval_step)

def accuracy_output_transform(output):
    y_pred, y = output
    preds = torch.round(torch.sigmoid(y_pred)).view(-1).long()
    targets = y.view(-1).long()
    return preds, targets

def loss_output_transform(output):
    y_pred, y = output
    return y_pred.view(-1), y.view(-1).float()

Accuracy(output_transform=accuracy_output_transform).attach(evaluator_train, "accuracy")
Loss(criterion, output_transform=loss_output_transform).attach(evaluator_train, "loss")

Accuracy(output_transform=accuracy_output_transform).attach(evaluator_val, "accuracy")
Loss(criterion, output_transform=loss_output_transform).attach(evaluator_val, "loss")

@trainer.on(Events.EPOCH_COMPLETED)
def run_validation(engine):
    evaluator_train.run(train_loader)
    train_m = evaluator_train.state.metrics
    evaluator_val.run(val_loader)
    val_m = evaluator_val.state.metrics
    print(f"Epoch {engine.state.epoch} | Train Loss: {train_m['loss']:.4f} | Train Acc: {train_m['accuracy']*100:.2f}% | Val Loss: {val_m['loss']:.4f} | Val Acc: {val_m['accuracy']*100:.2f}%")

trainer.run(train_loader, max_epochs=10)

model_path = os.path.abspath(os.path.join(os.path.curdir, "model.pth"))
torch.save(model.state_dict(), model_path)
export_model_path = os.path.abspath(os.path.join(os.path.curdir, "model.onnx"))
sample_input = torch.randn(1, 4)
torch.onnx.export(
    model,
    sample_input,
    export_model_path,
    export_params=True,
    opset_version=18
)
print("ONNX model saved to: {}".format(export_model_path))
