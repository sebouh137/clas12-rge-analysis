import functions_RC

def main():
    ScatteredElectron_Energy = 3.3
    ScatteredElectron_Theta  = 15.0
    rc_factor = functions_RC.get_rc_ratio("carbon12", ScatteredElectron_Energy, ScatteredElectron_Theta)
    cc_factor = functions_RC.get_cc_ratio("carbon12", ScatteredElectron_Energy, ScatteredElectron_Theta)

    print("The RC factor is {}".format(rc_factor))
    print("The CC factor is {}".format(cc_factor))

if __name__ == "__main__":
    main()
