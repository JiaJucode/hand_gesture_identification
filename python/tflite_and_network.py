import pandas as pd
import numpy as np
import tensorflow as tf
import socket
import sys
import struct
import time
from tensorflow.keras import layers

#build interpreter and load model
interpreter = tf.lite.Interpreter(model_path=sys.argv[1])
interpreter.allocate_tensors()

# Get input and output tensors.
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()


#connect to mediapipe graph
HOST = "127.0.0.1"
PORT = int(sys.argv[2]) #59832 for thumb
INPUT_SIZE = 25
OUTPUT_SIZE = 2
FLOAT_SIZE = 4

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    while (True):
        input = s.recv(INPUT_SIZE * FLOAT_SIZE)
        if not input or len(input) != INPUT_SIZE * FLOAT_SIZE:
            break
        input = struct.unpack('<25f', input)
        print(input)
        input_shape = input_details[0]['shape']
        interpreter.set_tensor(input_details[0]['index'], [input])
        interpreter.invoke()
        output_data = interpreter.get_tensor(output_details[0]['index'])
        print(output_data)
        s.sendall(output_data)
        