import pandas as pd
import os
import matplotlib.pyplot as plt
print "Enter location of file"
location=raw_input()
os.chdir(location)
print "Enter a filename"
file_name=raw_input()

df = pd.DataFrame.from_csv(file_name, parse_dates=False)
df.P.plot(color='g',lw=1)
df.V.plot(color='b',lw=1)
#df.b.plot(color='y',lw=1.3)
plt.xlabel('Time')
plt.legend()
plt.show()
