import numpy as np
import matplotlib.pyplot as plt
from scipy import signal



dB40 = 33
dB80 = 30

test = [16, 18, 48, 50]


ephys = np.fromfile('rhd2164-ephys_2024-01-25T16_50_00.raw', dtype=np.uint16).reshape(-1,64)

plt.close('all)')
plt.plot(ephys[:, dB80], 'k')
plt.plot(ephys[:, 16])
plt.plot(ephys[:, 18])
plt.plot(ephys[:, 48])
plt.plot(ephys[:, 50])

#%%

plt.close('all')
fs = 30e3
f, Pxx_den = signal.welch(ephys[:, dB80], fs, nperseg=1024)
plt.semilogy(f, Pxx_den)

f, Pxx_den = signal.welch(ephys[:, 18], fs, nperseg=1024)
plt.semilogy(f, Pxx_den)

f, Pxx_den = signal.welch(ephys[:, 16], fs, nperseg=1024)
plt.semilogy(f, Pxx_den)

plt.ylim([0.5e-3, 10])
plt.title("100 Hz sine wave from teensy with ~4 kHz update")
plt.xlabel('Frequency ($Hz$)')
plt.ylabel('PSD ($V^{2}/Hz$)')
plt.legend(["No analog switch", "Attenuation after analog switch", "Attenuation before analog switch"])
plt.show()





 