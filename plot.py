import matplotlib.pyplot as plt
import csv

# Prepare storage
names = []
insert_times = []
lookup_times = []
vmpeaks = []
vmrsses = []

# Read CSV manually
with open('benchmark.csv', 'r') as f:
    reader = csv.reader(f)
    next(reader)  # Skip header line
    for row in reader:
        names.append(row[0])
        insert_times.append(float(row[1]))
        lookup_times.append(float(row[2]))
        vmpeaks.append(int(row[3]))
        vmrsses.append(int(row[4]))

# Set x positions
x = range(len(names))
bar_width = 0.35

# Plot Insert Time and Lookup Time
plt.figure(figsize=(10, 6))
plt.bar(x, insert_times, width=bar_width, label='Insert Time (s)', align='center')
plt.bar([i + bar_width for i in x], lookup_times, width=bar_width, label='Lookup Time (s)', align='center')
plt.xticks([i + bar_width/2 for i in x], names, rotation=20, ha='right')
plt.ylabel('Time (seconds in log scale)')
plt.yscale('log')
plt.title('Benchmark Insert vs Lookup Time')
plt.legend()
plt.grid(axis='y', which='both')
plt.tight_layout()
plt.show()

# Plot Memory Usage (VmRSS and VmPeak)
plt.figure(figsize=(10, 6))
plt.bar(x, vmrsses, width=bar_width, label='VmRSS (kB)', align='center')
plt.bar([i + bar_width for i in x], vmpeaks, width=bar_width, label='VmPeak (kB)', align='center')
plt.xticks([i + bar_width/2 for i in x], names, rotation=20, ha='right')
plt.ylabel('Memory (kB)')
plt.title('Benchmark Memory Usage')
plt.legend()
plt.grid(axis='y')
plt.tight_layout()
plt.show()