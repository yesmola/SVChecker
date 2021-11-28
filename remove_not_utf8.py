import sys

def main():
    if len(sys.argv) != 3:
        print("Usage: python remove_not_utf8.py [file_in] [file_out]")
        exit()
    filein = sys.argv[1]
    fileout = sys.argv[2]
    with open(filein, 'rb') as csv_in:
        with open(fileout, "w", encoding="utf-8") as csv_temp:
            for line in csv_in:
                if not line:
                    break
                else:
                    line = line.decode("utf-8", "ignore")
                    csv_temp.write(str(line).rstrip() + '\n')

if __name__ == "__main__":
    main()