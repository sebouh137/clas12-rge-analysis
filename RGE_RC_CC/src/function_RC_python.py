import numpy as np
from scipy.interpolate import griddata

def import_radcor(filename):
    data = []
    with open(filename, 'r') as file:
        for line in file:
            values = list(map(float, line.split()))
            data.append(values)
    columns = zip(*data)
    return [list(column) for column in columns]

def find_nearest_value(input_value, angle, angle_deviation=6.0):
    valid_values = np.arange(angle - angle_deviation, angle + angle_deviation, 0.2)
    if input_value < valid_values.min():
        print("Input value {} is too small. Minimum allowed value is {}.".format(input_value, valid_values.min()))
        return valid_values.min()
    if input_value > valid_values.max():
        print("Input value {} is too large. Maximum allowed value is {}.".format(input_value, valid_values.max()))
        return valid_values.max()
    nearest_value = valid_values[np.abs(valid_values - input_value).argmin()]
    return nearest_value

def linear_interpolation(x0, y0, x1, y1, x):
    return y0 + (y1 - y0) * (x - x0) / (x1 - x0)

def are_equal(a, b, tolerance=1e-8):
    return np.abs(a - b) < tolerance

def interpolate_1d_from_graph2d(x_values, y_values, z_values, eprime, angle):
    points = [(x, z) for x, y, z in zip(x_values, y_values, z_values) if are_equal(y, angle)]
    if len(points) < 2:
        print("Error: Not enough points for interpolation at angle {}".format(angle))
        return np.nan
    points = sorted(points)
    x_vals, z_vals = zip(*points)
    if eprime < x_vals[0] or eprime > x_vals[-1]:
        print("Error: Interpolation point is outside the range of the data")
        return np.nan
    index = np.searchsorted(x_vals, eprime)
    x0, y0 = x_vals[index - 1], z_vals[index - 1]
    x1, y1 = x_vals[index], z_vals[index]
    return linear_interpolation(x0, y0, x1, y1, eprime)

def create_graph_2d(v2, v3, vdata):
    return np.array([v2, v3, vdata])

def extract_rc_factor(graph, eprime, thetadeg):
    x_values, y_values, z_values = graph
    return interpolate_1d_from_graph2d(x_values, y_values, z_values, eprime, thetadeg)

def get_rc_ratio(target, eprime, thetadeg):
    v1_lt, v2_lt, v3_lt, v4_lt, v5_lt, v6_lt, v7_lt, v8_lt, v9_lt, v10_lt, v11_lt, v12_lt, v13_lt = import_radcor("cryo2.out")
    v1_st, v2_st, v3_st, v4_st, v5_st, v6_st, v7_st, v8_st, v9_st, v10_st, v11_st, v12_st, v13_st = import_radcor("{}.out".format(target))
    
    graph2d_sigma_rad_st = create_graph_2d(v2_st, v3_st, v9_st)
    graph2d_sigma_born_st = create_graph_2d(v2_st, v3_st, v6_st)
    graph2d_sigma_rad_lt = create_graph_2d(v2_lt, v3_lt, v9_lt)
    graph2d_sigma_born_lt = create_graph_2d(v2_lt, v3_lt, v6_lt)
    graph2d_coulomb_correction_st = create_graph_2d(v2_st, v3_st, v13_st)

    sigma_rad_lt = extract_rc_factor(graph2d_sigma_rad_lt, eprime, thetadeg)
    sigma_rad_st = extract_rc_factor(graph2d_sigma_rad_st, eprime, thetadeg)
    sigma_born_lt = extract_rc_factor(graph2d_sigma_born_lt, eprime, thetadeg)
    sigma_born_st = extract_rc_factor(graph2d_sigma_born_st, eprime, thetadeg)

    factor = (sigma_rad_st / sigma_born_st) / (sigma_rad_lt / sigma_born_lt)
    return factor

def get_cc_ratio(target, eprime, thetadeg):
    v1_st, v2_st, v3_st, v4_st, v5_st, v6_st, v7_st, v8_st, v9_st, v10_st, v11_st, v12_st, v13_st = import_radcor("{}.out".format(target))
    graph2d_coulomb_correction_st = create_graph_2d(v2_st, v3_st, v13_st)
    return extract_rc_factor(graph2d_coulomb_correction_st, eprime, thetadeg)

def main():
    rc_factor = get_rc_ratio("carbon12", 3.3, 15.0)
    cc_factor = get_cc_ratio("carbon12", 3.3, 15.0)

    print("The RC factor is {}".format(rc_factor))
    print("The CC factor is {}".format(cc_factor))

if __name__ == "__main__":
    main()
