import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, TensorDataset
from sklearn.model_selection import train_test_split
from sklearn.datasets import make_classification
from sklearn.preprocessing import StandardScaler

from ignite.engine import Engine, Events, create_supervised_trainer, create_supervised_evaluator
from ignite.metrics import Accuracy, Loss

import pandas as pd

from dataset import DNSExfiltrationDataset

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

device = "cuda" if torch.cuda.is_available() else "cpu"
model = BinaryClassifier().to(device)

train_dataset = DNSExfiltrationDataset('../dataset/training.csv')
val_dataset = DNSExfiltrationDataset('../dataset/validating.csv')

train_loader = DataLoader(train_dataset, batch_size=32, shuffle=True)
val_loader = DataLoader(val_dataset, batch_size=64, shuffle=True)

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
torch.save(model.state_dict(), "model.pth")

sample_input = torch.randn(1, 4)
onnx_program = torch.onnx.export(
    model,
    args=(sample_input,), 
    f="model.onnx", 
    dynamo=True,
    opset_version=19
)
print("Model saved!")
