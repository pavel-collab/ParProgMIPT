import numpy as np

def CleanFile(file_path):
    fd = open(file_path, "w")
    fd.write("")
    fd.close()

def GenArray(low, high, size, file_path):
    CleanFile(file_path)

    gen_array = np.random.randint(low, high, size)
    gen_array_size = np.size(gen_array)
    
    fd = open(file_path, "w")
    fd.write(str(gen_array_size) + "\n")
    for item in gen_array:
        fd.write(str(item) + " ")
    fd.close()

def GenBadCase(low, high, size, file_path):
    CleanFile(file_path)

    gen_array = np.random.randint(low, high, size)
    gen_array_size = np.size(gen_array)

    gen_array = list(gen_array)
    gen_array.sort(reverse=True) 
    
    fd = open(file_path, "w")
    fd.write(str(gen_array_size) + "\n")
    for i in range(gen_array_size-1):
        fd.write(str(gen_array[i]) + ", ")
    fd.write(str(gen_array[-1]))
    fd.close()

def main():
    low = -2048
    high = 2048
    file_path = "../tests/"

    
if __name__ == '__main__':
    main()