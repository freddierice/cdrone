import logz
import matplotlib.pyplot as plt

def main():
    """main."""
    with logz.open("run.logz") as l:
        rc = l["rc"]
        plt.plot(rc["time"], rc["thrust"])
        plt.show()

if __name__ == '__main__':
    main()
