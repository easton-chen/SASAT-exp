import keras
import numpy as np


INTERNALDATA_PATH = "./internal_data.txt"
EXTERNALDATA_PATH = "./external_data.txt"

IDName = './model/internal_data_model.h5'
EDName1 = './model/external_data_model1.h5'
EDName2 = './model/external_data_model2.h5'
EDName3 = './model/external_data_model3.h5'
EDName4 = './model/external_data_model4.h5'



#input : 1, [2,3,4] ----> input_numpy_array = np.array([[1,2,3,4]])
def use_model(input_numpy_array, model_name):
    model_tmp = keras.models.load_model(model_name)
    return(model_tmp.predict(input_numpy_array))


'''
example:

X = []
X = np.array([[12,0.8800000000000004, 0.15999949999776, 0.059999999999999776]])
print(use_model(X, IDName))

'''
