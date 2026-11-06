import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt

# test data. taken from RC circuit simulation in LTspice
points = [
    (0, 4.98), (0.000005, 4.98), (0.000010, 3.01), (0.000020, 1.847),
    (0.000040, 0.681),  (0.000080, 0.09), (0.000160, 0.004),
]

x_raw = np.array([p[0] for p in points], dtype=float)
y_raw = np.array([p[1] for p in points], dtype=float)

index = []

# eliminate repeated data points (so that flat part of curve doesn't affect model)
for i in range(len(x_raw)):
    if i == 0:
        continue
    if y_raw[i] == y_raw[i - 1]:
        index.append(i-1)
    else:
        delay = (x_raw[i] - x_raw[i - 1])

x = np.delete(x_raw, index)
y = np.delete(y_raw, index)

if np.any(y <= 0):
    raise ValueError("All y values must be positive for an exponential fit.")

# initial guess via linear fit on ln(y) = ln(K) + (1/c) * x
slope, intercept = np.polyfit(x, np.log(y), 1)
K0, b0 = np.exp(intercept), slope          # b = 1/c

# nonlinear least squares on y = K * exp(b * x)
# (fitting b = 1/c avoids a singularity if c is huge or the data is flat)
def model(x, K, b):
    return K * np.exp(b * x)

(K, b), _ = curve_fit(model, x, y, p0=(K0, b0))
c = - 1 / b

# check how well the model fits the data
y_pred = model(x, K, b)
r2 = 1 - np.sum((y - y_pred) ** 2) / np.sum((y - y.mean()) ** 2)

print(f"V_max  = {K:.5f}")
print(f"Tau  = {c:.5f} ")
print(f"R² = {r2:.5f}")
print(f"Delay = {delay:.5f} ")
print(f"Best fit: y = {K:.4f} * e^(x/{c:.4f})")
print("Guess: Magnetic")

# Plot
xs = np.linspace(x_raw.min(), x_raw.max(), 300)
plt.scatter(x_raw, y_raw, color="red", label="Data")
plt.plot(xs, model(xs, K, b), label=f"y = {K:.3f}·e^(x/{c:.3f})")
plt.xlabel("x"); plt.ylabel("y"); plt.legend(); plt.grid(True)
plt.show()

# idea. treat curve as sum of decaying exponentials. more accurate according to physics. might implement it
# compare t_late and t_early
# tau = (t_b - t_a) / ln(V_a/V_b)