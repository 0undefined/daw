import numpy as np
import matplotlib.pyplot as plt

fig = plt.figure()

def f(x):
    # experience to level
    #return (np.log(x/4+1)**2) / 4 + 1
    # Difficulty scaling
    return 1 + (2**(np.log(1 + x)) + np.floor(x / 5) / 2) * 100
    # Light level
    #return np.log(1+x) / np.log(2) * (4*log(2)/log(3))

x = np.arange(25)
y = [f(i)/15 for i in x]


cutoff = len([v for v in y if v >= 0])
#
#
plt.grid(visible=True, which='major', axis='both')
plt.plot(x[:cutoff], y[:cutoff])

plt.show()

for v in zip(x,y):
    print('{}:{:.2f}'.format(v[0], v[1]))
