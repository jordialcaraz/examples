import argparse
 
import numpy as np
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
 
 
def build_model():
    # Same architecture as the PyTorch MNIST example
    return keras.Sequential([
        keras.Input(shape=(28, 28, 1)),
        layers.Conv2D(32, 3, activation="relu"),
        layers.Conv2D(64, 3, activation="relu"),
        layers.MaxPooling2D(2),
        layers.Dropout(0.25),
        layers.Flatten(),
        layers.Dense(128, activation="relu"),
        layers.Dropout(0.5),
        layers.Dense(10),
    ])
 
 
def load_data():
    (x_train, y_train), (x_test, y_test) = keras.datasets.mnist.load_data()
    # Same normalization as the PyTorch example
    x_train = ((x_train.astype("float32") / 255.0 - 0.1307) / 0.3081)[..., np.newaxis]
    x_test = ((x_test.astype("float32") / 255.0 - 0.1307) / 0.3081)[..., np.newaxis]
    return (x_train, y_train), (x_test, y_test)
 
 
def main():
    parser = argparse.ArgumentParser(description="TensorFlow MNIST Example")
    parser.add_argument("--batch-size", type=int, default=64)
    parser.add_argument("--test-batch-size", type=int, default=1000)
    parser.add_argument("--epochs", type=int, default=14)
    parser.add_argument("--lr", type=float, default=1.0)
    parser.add_argument("--gamma", type=float, default=0.7,
                        help="learning rate decay factor applied every epoch")
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--no-accel", action="store_true",
                        help="hide GPUs and run on CPU only")
    parser.add_argument("--eager", action="store_true",
                        help="run eagerly instead of building a tf.function graph")
    parser.add_argument("--dry-run", action="store_true",
                        help="quickly check a single pass")
    parser.add_argument("--save-model", action="store_true")
    args = parser.parse_args()
 
    if args.no_accel:
        tf.config.set_visible_devices([], "GPU")
 
    keras.utils.set_random_seed(args.seed)
 
    (x_train, y_train), (x_test, y_test) = load_data()
 
    model = build_model()
    model.compile(
        optimizer=keras.optimizers.Adadelta(learning_rate=args.lr),
        loss=keras.losses.SparseCategoricalCrossentropy(from_logits=True),
        metrics=["accuracy"],
        run_eagerly=args.eager,
    )
 
    # StepLR(step_size=1, gamma) equivalent
    lr_schedule = keras.callbacks.LearningRateScheduler(
        lambda epoch: args.lr * (args.gamma ** epoch))
 
    epochs = 1 if args.dry_run else args.epochs
    model.fit(
        x_train, y_train,
        batch_size=args.batch_size,
        epochs=epochs,
        steps_per_epoch=1 if args.dry_run else None,
        validation_data=(x_test, y_test),
        validation_batch_size=args.test_batch_size,
        callbacks=[lr_schedule],
        verbose=2,
    )
 
    if args.save_model:
        model.save("mnist_cnn.keras")
 
 
if __name__ == "__main__":
    main()

