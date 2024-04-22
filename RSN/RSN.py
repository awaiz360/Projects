import torch
import torch.nn as nn
import torch.optim as optim
import torchvision.transforms as transforms
from torchvision.datasets import MNIST
from torch.utils.data import DataLoader
import matplotlib.pyplot as plt

# Set random seed for reproducibility
torch.manual_seed(42)

# Define the Reservoir Neural Network (RSN) model
class RSN(nn.Module):
    def __init__(self, input_size, reservoir_size, num_classes):
        super(RSN, self).__init__()
        self.reservoir = nn.Linear(input_size, reservoir_size, bias=False)
        self.output_layer = nn.Linear(reservoir_size, num_classes)

    def forward(self, x):
        x = torch.tanh(self.reservoir(x))
        x = self.output_layer(x)
        return x

# Load MNIST dataset
transform = transforms.Compose([transforms.ToTensor(), transforms.Normalize((0.5,), (0.5,))])
train_dataset = MNIST(root='./data', train=True, transform=transform, download=True)
test_dataset = MNIST(root='./data', train=False, transform=transform, download=True)

# Set hyperparameters
input_size = 28 * 28  # 784 pixels
reservoir_size = 200  # Number of reservoir nodes
num_classes = 10      # Digits 0 to 9
batch_size = 64
learning_rate = 0.001
num_epochs = 10

# Create data loaders
train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)
test_loader = DataLoader(test_dataset, batch_size=batch_size, shuffle=False)

# Initialize the RSN model
model = RSN(input_size, reservoir_size, num_classes)

# Define the loss function and optimizer
criterion = nn.CrossEntropyLoss()
optimizer = optim.Adam(model.parameters(), lr=learning_rate)

# Training the RSN model
for epoch in range(num_epochs):
    model.train()
    total_loss = 0.0
    for inputs, labels in train_loader:
        inputs = inputs.view(-1, input_size)  # Flatten the input images
        optimizer.zero_grad()
        outputs = model(inputs)
        loss = criterion(outputs, labels)
        loss.backward()
        optimizer.step()
        total_loss += loss.item()

    print(f'Epoch [{epoch + 1}/{num_epochs}], Loss: {total_loss / len(train_loader)}')

# Evaluation on the test set and visualization
model.eval()
correct = 0
total = 0
with torch.no_grad():
    for inputs, labels in test_loader:
        inputs = inputs.view(-1, input_size)
        outputs = model(inputs)
        _, predicted = torch.max(outputs.data, 1)
        total += labels.size(0)
        correct += (predicted == labels).sum().item()

        # Visualize the input and output images
        for i in range(len(inputs)):
            image = inputs[i].view(28, 28) * 0.5 + 0.5
            plt.subplot(1, 2, 1)
            plt.imshow(image, cmap='gray')
            plt.title("Input Image")
            plt.axis('off')

            output_prob = torch.softmax(outputs, dim=1)[i]
            plt.subplot(1, 2, 2)
            plt.bar(range(num_classes), output_prob)
            plt.title("Output Probabilities")
            plt.xticks(range(num_classes))
            plt.xlabel('Digit Class')
            plt.ylabel('Probability')
            plt.tight_layout()
            plt.show()

print(f'Accuracy on the test set: {100 * correct / total}%')
