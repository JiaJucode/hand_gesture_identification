# hand_gesture_identification

--onging project

Running:  
1. git clone mediapipe library: git clone https://github.com/google/mediapipe.git  
2. copy files except python into the mediapipe repository under: //mediapipe/mediapipe/examples/  
3. build with "bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/c++:__file_to_build__"  
4. run with "bazel-bin/mediapipe/examples/c++/__file_to_run__"  
5. run python in a new command line terminal, details for running python in python/run_python.txt  
