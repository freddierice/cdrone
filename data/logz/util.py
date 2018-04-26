import numpy as np

def bin_up(a, b):
    """bin_up bins to the longest arr."""
    if len(a) > len(b):
        (new_a, new_b) = _bin_up(a, b)
    else:
        (new_b, new_a) = _bin_up(b, a)

    return (new_a, new_b)

def _bin_up(a, b):
    """bin up to a."""
    dtype_b = np.dtype([x for x in b.dtype.descr if not x[0] == 'time'])
    new_b = np.ndarray((len(a),), dtype=b.dtype.descr)

    idx = 0
    b_time = b['time']
    new_b_time = a['time']
    for i in range(len(b)):
        while idx < len(a) and new_b_time[idx] <= b_time[i]:
            new_b[idx] = b[i]
            idx += 1
    new_b['time'][:] = a['time'][:]
    
    return (a.copy(), new_b)

def bin_vars(a, b):
    """bin two arrays by their time."""
    if len(a) > len(b):
        (new_time, new_a, new_b) = _bin_vars(a, b)
    else:
        (new_time, new_b, new_a) = _bin_vars(b, a)

    return (new_time, new_a, new_b)

EXPAND_SIZE=10
def _bin_vars(a, b):
    """bin two arrays by their time where len(a) >= len(b)."""
    new_size = len(a) * EXPAND_SIZE
    dtype_a = np.dtype([x for x in a.dtype.descr if not x[0] == 'time'])
    dtype_b = np.dtype([x for x in b.dtype.descr if not x[0] == 'time'])
    min_time = min(a[0]['time'], b[0]['time'])
    max_time = min(a[-1]['time'], b[-1]['time'])
    time_inc = float(max_time - min_time) / new_size

    # create a combined object with a single time
    combined_time = np.ndarray(shape=(new_size,), dtype='<u8')
    combined_a = np.ndarray(shape=(new_size,), dtype=dtype_a)
    combined_b = np.ndarray(shape=(new_size,), dtype=dtype_b)
    combined_time[:] = np.array([int(x*time_inc)+min_time for x in range(new_size)], dtype='<u8')[:]

    # first copy  in a
    idx = 0
    a_time = a['time']
    a_type = a[[x[0] for x in dtype_a.descr]]
    for i in range(len(a)):
        while idx < new_size and combined_time[idx] <= a_time[i]:
            combined_a[idx] = a_type[i]
            idx += 1

    idx = 0
    b_time = b['time']
    b_type = b[[x[0] for x in dtype_b.descr]]
    for i in range(len(b)):
        while idx < new_size and combined_time[idx] <= b_time[i]:
            combined_b[idx] = b_type[i]
            idx += 1

    return (combined_time, combined_a, combined_b)

def dt(x,var):
    time = x['time']
    val = x[var]
    a = val.copy()

    a = a[:-1]
    for i in range(len(a)):
        a[i] = 10000*(val[i+1] - val[i]) / (time[i+1] - time[i] + 0.00000000000001)
    return a
