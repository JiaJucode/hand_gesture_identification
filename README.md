# hand_gesture_identification

--onging project

Running requirements:  
--install bazel https://docs.bazel.build/versions/main/install.html  
--git clone https://google.github.io/mediapipe/getting_started/install.html  
  
Running:
1. copy files except python into the mediapipe repository under: //mediapipe/mediapipe/examples/  
2. build with "bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/c++:__file_to_build__"  
3. run with "bazel-bin/mediapipe/examples/c++/__file_to_run__"  
4. run python in a new command line terminal, details for running python in python/run_python.txt  
