import numpy as np
import matplotlib.pyplot as plt

# Create the validation dataset
np.random.seed(0)

# Number of points
N = 10000

# Radius of the unitary circle
radius = 1

# Generate points inside the unitary circle
theta_inside = 2 * np.pi * np.random.rand(N)
x_inside = radius * np.cos(theta_inside)
y_inside = radius * np.sin(theta_inside)

# Generate points outside the unitary circle but inside the square
L = np.sqrt(2 * np.pi)
x_outside = L * (2 * np.random.rand(N) - 1)
y_outside = L * (2 * np.random.rand(N) - 1)

# Plot the dataset
plt.figure(figsize=(8, 8))
plt.scatter(x_inside, y_inside, c='yellow', label='Class 1 (Inside Circle)')
plt.scatter(x_outside, y_outside, c='purple', label='Class 2 (Outside Circle)')
#plt.plot(radius * np.cos(np.linspace(0, 2 * np.pi, 100)),
 #        radius * np.sin(np.linspace(0, 2 * np.pi, 100)), 'r', label='Unitary Circle')
plt.xlim(-1.25, 1.25)
plt.ylim(-1.25, 1.25)
plt.xlabel('x')
plt.ylabel('y')
plt.title('Validation Dataset for RSN')
plt.legend()
plt.grid(True)
plt.show()

import numpy as np
import tensorflow as tf
from tensorflow.keras.layers import Input, Dense
from tensorflow.keras.models import Model
from tensorflow.keras.optimizers import Adam

# Combine the dataset points and labels
x_train = np.vstack((np.vstack((x_inside, y_inside)).T, np.vstack((x_outside, y_outside)).T))
y_train = np.hstack((np.ones(N), np.zeros(N)))

# Shuffle the dataset
indices = np.arange(2 * N)
np.random.shuffle(indices)
x_train = x_train[indices]
y_train = y_train[indices]


# Define the RSN model
def create_rsn_model():
    input_layer = Input(shape=(2,))
    x = Dense(20, activation='tanh')(input_layer)
    x = Dense(20, activation='tanh')(x)
    output_layer = Dense(1, activation='sigmoid')(x)

    model = Model(inputs=input_layer, outputs=output_layer)
    return model


rsn_model = create_rsn_model()
rsn_model.compile(loss='binary_crossentropy', optimizer=Adam(lr=0.001), metrics=['accuracy'])

# Train the RSN model
history = rsn_model.fit(x_train, y_train, epochs=100, batch_size=32)

# Plot the accuracy and loss during training
plt.figure(figsize=(10, 5))
plt.subplot(1, 2, 1)
plt.plot(history.history['accuracy'])
plt.xlabel('Epoch')
plt.ylabel('Accuracy')
plt.title('Training Accuracy')

plt.subplot(1, 2, 2)
plt.plot(history.history['loss'])
plt.xlabel('Epoch')
plt.ylabel('Loss')
plt.title('Training Loss')

plt.tight_layout()
plt.show()

import numpy as np
import tensorflow as tf
from tensorflow.keras.datasets import mnist
from tensorflow.keras.layers import Input, Dense
from tensorflow.keras.models import Model
from tensorflow.keras.optimizers import Adam

# Load MNIST dataset
(train_images, train_labels), (test_images, test_labels) = mnist.load_data()

# Normalize and flatten images
train_images = train_images.reshape(-1, 28*28) / 255.0
test_images = test_images.reshape(-1, 28*28) / 255.0

# Define parameters
N = 784  # Number of nodes in the RSN
N0 = 28*28  # Number of input nodes
epsilon = 0.1  # Value for parameter 𝜖
k_max = 10  # Maximum iteration for training (𝑘̄)

# Split MNIST dataset into two subsets: digits 0 to 4 and 5 to 9
train_indices_0_to_4 = np.where(train_labels < 5)[0]
train_indices_5_to_9 = np.where(train_labels >= 5)[0]

train_images_0_to_4 = train_images[train_indices_0_to_4]
train_labels_0_to_4 = train_labels[train_indices_0_to_4]

train_images_5_to_9 = train_images[train_indices_5_to_9]
train_labels_5_to_9 = train_labels[train_indices_5_to_9]

# Define RSN model architecture
input_layer = Input(shape=(N0,))
hidden_layer = Dense(N, activation='tanh')(input_layer)
output_layer = Dense(10, activation='softmax')(hidden_layer)

# Create the RSN model
rsn_model = Model(inputs=input_layer, outputs=output_layer)

# Compile the model
rsn_model.compile(optimizer=Adam(), loss='sparse_categorical_crossentropy', metrics=['accuracy'])

# First Training Round: Train RSN on digits 0 to 4
rsn_model.fit(train_images_0_to_4, train_labels_0_to_4, epochs=k_max)

# Set eigenvalues of the second set to zero
# (This is a simplified version as full details for setting eigenvalues are not provided)
eigenvalues_second_set = np.ones(N//2)
eigenvalues_second_set[:5] = 0

# Second Training Round: Train RSN on digits 5 to 9
rsn_model.fit(train_images_5_to_9, train_labels_5_to_9, epochs=k_max)

# Set eigenvalues of the first set to zero
# (This is a simplified version as full details for setting eigenvalues are not provided)
eigenvalues_first_set = np.ones(N//2)
eigenvalues_first_set[:5] = 0

# Set eigenvalues in the RSN model (This step requires custom implementation based on full details from the paper)
# You may need to implement the manipulation of eigenvalues and eigenvectors in the RSN model.

# Evaluate the RSN model on both datasets
test_loss_0_to_4, test_accuracy_0_to_4 = rsn_model.evaluate(test_images, test_labels, verbose=0)
print("Test accuracy for digits 0 to 4:", test_accuracy_0_to_4)

test_loss_5_to_9, test_accuracy_5_to_9 = rsn_model.evaluate(test_images, test_labels, verbose=0)
print("Test accuracy for digits 5 to 9:", test_accuracy_5_to_9)
