import numpy as np
import tensorflow as tf


class RecurrentSpectralNetwork:
    def __init__(self, N, n, k_max=100, learning_rate=0.01, epochs=200):
        self.N = N
        self.n = n
        self.k_max = k_max
        self.learning_rate = learning_rate
        self.epochs = epochs
        self.k_bar = 60

        self._build_model()

    def _g_function(self, x):
        # Define the non-linear function (you may use different activation functions here)
        return tf.nn.relu(x)

    def _build_model(self):
        self.Phi = tf.Variable(tf.random.uniform(shape=(self.N, self.N - self.n), minval=-1, maxval=1))
        self.Lambda = tf.Variable(tf.random.uniform(shape=(self.N - self.n, 1), minval=0.5, maxval=1))
        self.g_layer = tf.keras.layers.Dense(30, activation='tanh')

        self.optimizer = tf.keras.optimizers.Adam(learning_rate=self.learning_rate)

    def _rsn_model(self, x, k):
        for _ in range(k):
            Ax = tf.matmul(self.Phi, x)
            x = 1 + self._g_function(Ax) / 1.5

        return x

    def _compute_coefficients(self, x, k_F, k_I):
        c_sum = tf.zeros(shape=(self.N, 1), dtype=tf.float32)
        for k in range(k_I, k_F + 1):
            x_k = self._rsn_model(x, k)
            c_k = tf.linalg.matmul(tf.linalg.inv(self.Phi), x_k)
            c_sum += c_k

        return c_sum / (k_F - k_I + 1)

    def train(self, train_data, train_labels):
        for epoch in range(self.epochs):
            with tf.GradientTape() as tape:
                loss = 0.0
                for i in range(len(train_data)):
                    x_0 = tf.expand_dims(train_data[i], axis=1)
                    k_F = np.random.randint(1, self.k_max)
                    k_I = np.random.randint(1, k_F)
                    c_avg = self._compute_coefficients(x_0, k_F, k_I)
                    loss += tf.reduce_sum(tf.square(c_avg - train_labels[i]))

                loss /= len(train_data)

            gradients = tape.gradient(loss, [self.Phi, self.Lambda, self.g_layer.weights])
            self.optimizer.apply_gradients(zip(gradients, [self.Phi, self.Lambda, self.g_layer.weights]))

            print(f"Epoch {epoch + 1}/{self.epochs}, Loss: {loss:.4f}")

    def classify_data(self, test_data):
        classifications = []
        for i in range(len(test_data)):
            x_0 = tf.expand_dims(test_data[i], axis=1)
            k_F = 100  # Choose a large value of k for final classification
            c_avg = self._compute_coefficients(x_0, k_F, self.k_bar)
            classifications.append(tf.argmax(c_avg).numpy())

        return classifications


# Dummy data for training and testing
N = 10
n = 2
train_data = np.random.rand(100, N)
train_labels = np.random.randint(0, n, size=(100,))
train_labels_one_hot = tf.one_hot(train_labels, depth=n, dtype=tf.float32)

test_data = np.random.rand(20, N)

# Create and train the RSN model
rsn_model = RecurrentSpectralNetwork(N, n)
rsn_model.train(train_data, train_labels_one_hot)

# Test the RSN model
classifications = rsn_model.classify_data(test_data)
print("Classifications:", classifications)
