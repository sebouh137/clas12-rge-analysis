import numpy as np
from scipy.interpolate import interp1d

# Function to import radiative correction data
def import_radcor(filename):
    data = np.loadtxt(filename)
    return data[:, 0], data[:, 1], data[:, 2], data[:, 3], data[:, 4], data[:, 5], data[:, 6], data[:, 7], data[:, 8], data[:, 9], data[:, 10], data[:, 11], data[:, 12]

# Function to find the nearest value
def find_nearest_value(input_val, angle, angle_deviation=6.0):
    min_value = angle - angle_deviation
    max_value = angle + angle_deviation

    valid_values = np.arange(min_value, max_value + 0.2, 0.2)

    if input_val < min_value:
        print("Input value {} is too small. Minimum allowed value is {}.".format(input_val, min_value))
        return min_value
    if input_val > max_value:
        print("Input value {} is too large. Maximum allowed value is {}.".format(input_val, max_value))
        return max_value

    nearest_value = valid_values[np.argmin(np.abs(valid_values - input_val))]
    return nearest_value

# Function for linear interpolation
def linear_interpolation(x0, y0, x1, y1, x):
    return y0 + (y1 - y0) * (x - x0) / (x1 - x0)

# Function to check if two values are equal within a tolerance
def are_equal(a, b, tolerance):
    return np.abs(a - b) < tolerance

# Function to interpolate 1D from graph 2D errors
def interpolate_1d_from_graph_2d_errors(graph, eprime, angle):
    x_values = []
    y_values = []
    z_values = []

    for i in range(len(graph)):
        x, y, z = graph[i]
        if are_equal(y, angle, 1e-8):
            x_values.append(x)
            y_values.append(y)
            z_values.append(z)

    if len(x_values) < 2:
        print("Error: Not enough points for interpolation at angle {}".format(angle))
        return np.nan

    points = sorted(zip(x_values, z_values))

    x_points, z_points = zip(*points)

    if eprime < x_points[0] or eprime > x_points[-1]:
        print("Error: Interpolation point is outside the range of the data")
        return np.nan

    interpolator = interp1d(x_points, z_points)
    return interpolator(eprime)

# Function to create a 2D graph
def create_graph_2d(v2, v3, vdata):
    return np.array(list(zip(v2, v3, vdata)))

# Function to extract RC factor
def extract_rc_factor(graph_2d_sigma_rad, eprime, thetadeg):
    return interpolate_1d_from_graph_2d_errors(graph_2d_sigma_rad, eprime, thetadeg)

if __name__ == "__main__":
    # INITIALIZING THE ARRAYS TO READ THE TABLES, WE HAVE TO DO THIS ONLY ONCE IN THE CODE.
    v1_lt, v2_lt, v3_lt, v4_lt, v5_lt, v6_lt, v7_lt, v8_lt, v9_lt, v10_lt, v11_lt, v12_lt, v13_lt = import_radcor("../bin/cryo2.out")
    v1_st, v2_st, v3_st, v4_st, v5_st, v6_st, v7_st, v8_st, v9_st, v10_st, v11_st, v12_st, v13_st = import_radcor("../bin/carbon12.out")

    gr2d_sigma_rad_st = create_graph_2d(v2_st, v3_st, v9_st)
    gr2d_sigma_born_st = create_graph_2d(v2_st, v3_st, v6_st)
    gr2d_sigma_rad_lt = create_graph_2d(v2_lt, v3_lt, v9_lt)
    gr2d_sigma_born_lt = create_graph_2d(v2_lt, v3_lt, v6_lt)

    # Just an example of values
    eprime = 3.3
    thetadeg = 15.0

    # Inside your for loop
    sigma_rad_lt = extract_rc_factor(gr2d_sigma_rad_lt, eprime, thetadeg)
    sigma_rad_st = extract_rc_factor(gr2d_sigma_rad_st, eprime, thetadeg)
    sigma_born_lt = extract_rc_factor(gr2d_sigma_born_lt, eprime, thetadeg)
    sigma_born_st = extract_rc_factor(gr2d_sigma_born_st, eprime, thetadeg)

    print("The RC factor (Rad to Born ratio) is: {} for D2, and {} for solid target.".format(sigma_rad_lt / sigma_born_lt, sigma_rad_st / sigma_born_st))
