import DataLoader, TensorDataset
from sklearn.model_selection import train_test_split
from sklearn.datasets import make_classification
from sklearn.preprocessing import StandardScaler

from ignite.engine import Engine, Events, create_supervised_trainer, create_supervised_evaluator
from ignite.metrics import Accuracy, Loss

import pandas as pd

class BinaryClassifier(nn.Module):
    def __init__(self):
        super(BinaryClassifier, self).__init__()
        self.layer_1 = nn.Linear(4, 16)
        self.layer_2 = nn.Linear(16, 8)
        self.layer_out = nn.Linear(8, 1)
        self.relu = nn.ReLU()
        
    def forward(self, x):
        return self.layer_out(self.relu(self.layer_2(self.relu(self.layer_1(x)))))

device = "cuda" if torch.cuda.is_available() else "cpu"
model = BinaryClassifier().to(device)

df = pd.read_csv('../dataset/training.csv')
X = df.iloc[:, [1]]  
y = df.iloc[:, 0]

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

train_dataset = TensorDataset(torch.tensor(X_train, dtype=torch.float32), torch.tensor(y_train, dtype=torch.float32).unsqueeze(1))
test_dataset = TensorDataset(torch.tensor(X_test, dtype=torch.float32), torch.tensor(y_test, dtype=torch.float32).unsqueeze(1))

train_loader = DataLoader(train_dataset, batch_size=32, shuffle=True)
test_loader = DataLoader(test_dataset, batch_size=64, shuffle=False)

criterion = nn.BCEWithLogitsLoss()
optimizer = optim.Adam(model.parameters(), lr=0.005)


def binary_output_transform(output):
    y_pred, y = output
    y_pred = torch.round(torch.sigmoid(y_pred)) # continuous to discrete
    return y_pred, y

trainer = create_supervised_trainer(model, optimizer, criterion, device=device)

val_metrics = {
    "accuracy": Accuracy(output_transform=binary_output_transform),
    "loss": Loss(criterion)
}
evaluator = create_supervised_evaluator(model, metrics=val_metrics, device=device)

@trainer.on(Events.EPOCH_COMPLETED)
def log_training_results(engine):
    epoch = engine.state.epoch
    if epoch % 20 == 0:
        evaluator.run(train_loader)
        metrics = evaluator.state.metrics
        print(f"Epoch {epoch:03d} | Train Loss: {metrics['loss']:.4f} | Train Acc: {metrics['accuracy']*100:.2f}%")

@trainer.on(Events.COMPLETED)
def log_final_results(engine):
    evaluator.run(test_loader)
    metrics = evaluator.state.metrics
    print(f"\n[Final Metrics] Test Loss: {metrics['loss']:.4f} | Test Accuracy: {metrics['accuracy']*100:.2f}%")

trainer.run(train_loader, max_epochs=100)

