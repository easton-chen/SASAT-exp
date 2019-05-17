from pandas import read_csv
from pandas import datetime
from matplotlib import pyplot
from statsmodels.tsa.arima_model import ARIMA
from sklearn.metrics import mean_squared_error
import math

def parser(x):
	return datetime.strptime('190'+x, '%Y-%m')

series = read_csv('clarknet-data.csv', header=0, parse_dates=[0], index_col=0, squeeze=True)
X = series.values

size = int(len(X) * 0.8)
train, test = X[0:size], X[size:len(X)]
history = [x for x in train]
predictions = list()
for t in range(len(test)):
	model = ARIMA(history, order=(9,1,0))
	model_fit = model.fit(disp=0)
	output = model_fit.forecast()
	yhat = output[0]
	predictions.append(yhat)
	obs = test[t]
	history.append(obs)
	print('predicted=%f, expected=%f' % (yhat, obs))
error = mean_squared_error(test, predictions)
RMSE = math.sqrt(error)
print('Test RMSE: %.3f' % RMSE)
# plot
pyplot.plot(test, label='true value')
pyplot.plot(predictions, color='orange', label='prediction')
pyplot.legend(loc='lower left')
pyplot.title('ARMA')
pyplot.show()