import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from matplotlib import rc
rc('text', usetex=True)

# csv file in form schedule,chunksize,num_threads,loop,time
results = pd.read_csv('results.csv')

# Take chunksize to log 2 using change of base rule
# This manipulation will also set chunksize for single and auto to be NAN
results.chunksize = np.rint(np.log(results.chunksize) / np.log(2))

# Find the mean time for each type of run
means = results.groupby(['schedule', 'chunksize', 'num_threads', 'loop']).mean()

# Find the standard deviation for each type of run
# Note: I will use this instead of the deviation in the mean, so that the user
# can see how the performance will vary run to run
stds = results.groupby(['schedule', 'chunksize', 'num_threads', 'loop']).std()

# Scale the stds to get the error in the mean
counts = results.groupby(['schedule', 'chunksize', 'num_threads', 'loop']).count()

# Return the data to its original data frame structure
means = means.reset_index()
stds = stds.reset_index()
counts = counts.reset_index()

## PLOTS for part 1 of the coursework

# Investigate the data for 6 threads
def six_threads():
  for loop in [1, 2]:
    data = []
    data_err = []
    colors = ['g', 'b', 'r']
    schedules = ['static', 'guided', 'dynamic']

    fig, ax = plt.subplots()
    for i, schedule in enumerate(schedules):
      data.append(means[(means.num_threads == 6) & (means.loop == loop) & (means.schedule == schedule)])
      data_err.append(stds[(stds.num_threads == 6) & (stds.loop == loop) & (stds.schedule == schedule)])

      data[i].plot(ax=ax, kind='scatter', x='chunksize', y='time', yerr=data_err[i], marker='_', c=colors[i], s=100, label=schedule)

    # Add auto into the graph
    auto_value = means[(means.num_threads == 6) & (means.loop == loop) & (means.schedule == 'auto')]['time'].values[0]
    ax.plot([0, 6], [auto_value, auto_value], color='k', linestyle='--', lw=2, label='auto')

    # Add static into the graph
    static_value = means[(means.num_threads == 6) & (means.loop == loop) & (means.schedule == 'static')]['time'].values[0]
    ax.plot([0, 6], [static_value, static_value], color='y', linestyle='--', lw=2, label='static')

    plt.title(rf"Runtimes of different schedules running on 6 threads for loop {loop}")
    plt.xlabel(r"\(log_2(chunksize)\)")
    plt.ylabel(r"\(time/s\)")
    plt.legend(loc='best')
    plt.savefig(f"./graphs/six_threads_loop_{loop}.png", dpi=500)

# Investigate the way each of the methods change with thread number
# With chunksize lines for each
def bonus_graphs():
  for loop in [1, 2]:
    data_a = means[(means.schedule == 'affinity') & (means.loop == loop)]

    for schedule in ['static', 'dynamic', 'guided']:
      fig, ax = plt.subplots()

      for chunksize in range(7):
        data = means[(means.schedule == schedule) & (means.chunksize == chunksize) & (means.loop == loop)]
        data_err = stds[(stds.schedule == schedule) & (stds.chunksize == chunksize) & (means.loop == loop)]

        color = [[chunksize/6, 0, 1 - chunksize/6]]
        data.plot(ax=ax, kind='scatter', x='num_threads', y='time', yerr=data_err,
                    marker='_', c=color, s=100, label=rf'\(chunksize, {2**chunksize}\)', logx=True)
      
      data_a.plot(ax=ax, kind='line', x='num_threads', y='time', c='k', label='affinity')

      plt.title(rf"Runtimes for {schedule} schedule running loop {loop}")
      plt.xlabel(r"\(Number of Threads\)")
      plt.ylabel(r"\(time/s\)")
      plt.savefig(f"./graphs/{schedule}_loop_{loop}.png", dpi=500)


def bonus_auto_static():
  for loop in [1, 2]:
    data_auto = means[(means.schedule == 'auto') & (means.loop == loop)]
    data_static = means[(means.schedule == 'static') & (means.loop == loop) & (np.logical_not(np.isfinite(means.chunksize)))]

    fig, ax = plt.subplots()
    data_auto.plot(ax=ax, kind='scatter', c='k', x='num_threads', y='time', label='auto', marker='+')
    data_static.plot(ax=ax, kind='scatter', c='y', x='num_threads', y='time', label='static', marker='x')

    plt.title(rf"Runtimes for auto and static schedules running loop {loop}")
    plt.xlabel(r"\(Number of Threads\)")
    plt.ylabel(r"\(time/s\)")
    plt.legend(loc='best')

    plt.savefig(f"./graphs/auto_static_loop_{loop}.png", dpi=500)


# Let's find the best scheduling option for 6 threads for each loop
loop1 = means[(means.loop == 1) & (means.num_threads == 6)]
loop2 = means[(means.loop == 2) & (means.num_threads == 6)]

# Find the minimum entry
min1 = loop1.loc[loop1['time'].idxmin()]
min2 = loop2.loc[loop2['time'].idxmin()]

print(f"""The best performance for loop 1 on 6 threads is achieved with {min1.schedule} schedule
  with a chunksize of {2**int(np.rint(min1.chunksize))} and an average execution time of {min1.time}""")

print(f"""The best performance for loop 2 on 6 threads is achieved with {min2.schedule} schedule
  with a chunksize of {2**int(np.rint(min2.chunksize))} and an average execution time of {min2.time}""")

# To make graphs of the speed up we need to find the reference time for the unparallel code
reference_1 = means[(means.schedule == 'single') & (means.loop == 1)].time
reference_2 = means[(means.schedule == 'single') & (means.loop == 2)].time

## note: It is not possible to find the standard deviation of the resultant ratio of random variables
# This is a general result

mins = [min1, min2]
references = [reference_1, reference_2]

def best_on_six():
  for loop in [1, 2]:
    fig, ax = plt.subplots()

    data = means[(means.schedule == mins[loop-1].schedule) & (means.chunksize == mins[loop-1].chunksize) & (means.loop == loop)].copy()

    # Rescale to get the speedup
    data.time = references[loop - 1].values[0] / data.time

    data.plot(ax=ax, kind='scatter', x='num_threads', y='time', marker='x', s=100, logx=True)

    plt.xlabel(r"\(Number of Threads\)")
    plt.ylabel(r'\(Speedup \frac{T_1}{T_p}\)')
    plt.title(rf"Performance of {mins[loop-1].schedule} using a chunksize of {2**int(np.rint(min2.chunksize))}")
    plt.savefig(f"./graphs/best_6_loop{loop}.png", dpi=500)


def affinity_graph():
  for loop in [1, 2]:
    data = means[(means.schedule == 'affinity') & (means.loop == loop)]
    data_err = stds[(stds.schedule == 'affinity') & (stds.loop == loop)]

    # Include the best of six data for reference
    data_2 = means[(means.schedule == mins[loop-1].schedule) & (means.chunksize == mins[loop-1].chunksize) & (means.loop == loop)].copy()
    data_2_err = stds[(stds.schedule == mins[loop-1].schedule) & (stds.chunksize == mins[loop-1].chunksize) & (means.loop == loop)].copy()


    fig, ax = plt.subplots()

    data.plot(ax=ax, kind='scatter', x='num_threads', y='time', yerr=data_err, marker='_', c='k', logx=True, label='affinity', s=50)
    data_2.plot(ax=ax, kind='scatter', x='num_threads', y='time', yerr=data_2_err, marker='_', s=50, logx=True, label=f"{mins[loop - 1].schedule}, {2**int(np.rint(mins[loop - 1].chunksize))}", c='b')
    plt.xlabel(r"\(Number of Threads\)")
    plt.ylabel(r"\(time/s\)")
    plt.title(rf"Performance of the affinity scheduling algorithm for loop{loop}")
    plt.legend(loc='best')
    plt.savefig(f"./graphs/affinity_loop{loop}.png", dpi=500)


six_threads()
bonus_graphs()
best_on_six()
bonus_auto_static()
affinity_graph()
