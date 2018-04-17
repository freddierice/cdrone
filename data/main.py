import logz

def main():
    """main."""
    with logz.open("run.logz") as l:
        print(l["rc"])
        print(l["rc"]["aux1"])

if __name__ == '__main__':
    main()
