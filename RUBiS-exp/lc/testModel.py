from load_model import *
import numpy as np
import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2' 

X = []
X = np.array([[2,1,0,200,500]])
print(X)
print(use_model(X, EDName1))
print(use_model(X, EDName2))
print(use_model(X, EDName3))
print(use_model(X, EDName4))
